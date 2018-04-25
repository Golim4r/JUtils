#ifndef JUTILS_H
#define JUTILS_H

#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

namespace J {

  /**************************FREQUENCY*******************************/
  //Iterations per Second
  class Frequency {
  public:
    
    Frequency(uint8_t smooth_over = 15) : smooth(smooth_over), second(1) { }
  
    void update(bool print_debug = false) {
      if (++count >= smooth) {
        count = 0;
        t_last = t_now;
        t_now = std::chrono::high_resolution_clock::now();
      
        time_span = std::chrono::duration<double, std::milli>(t_now - t_last);
      
        iterations_per_second = (iterations_per_second + ((second / time_span) * smooth)) / 2.f;
      
        if (print_debug) {
          if (std::chrono::duration<double>(t_now - t_out).count() > 1) {
            t_out = std::chrono::high_resolution_clock::now();
            std::cout << "Iterations per second: " << iterations_per_second << '\n';
          }
        }
      }
    }

    double ips() {
      return iterations_per_second;
    }
    
  private:
    uint8_t count = 0, smooth;
    double iterations_per_second;
    std::chrono::high_resolution_clock::time_point t_last, t_now, t_out;
    std::chrono::duration<double> time_span;
    std::chrono::seconds second;
  };
  
  /**************************DURATION*******************************/
  class Duration {
  public:
    void start() {
      t_checkpoints.clear();
      stopped = false;
      t_start = std::chrono::high_resolution_clock::now();
    }
    
    void checkpoint(std::string comment = "") {
      if (!stopped) {
        t_checkpoints.emplace_back(std::chrono::high_resolution_clock::now());
        comments.emplace_back(comment);
      }
    }
    
    void stop() {
      if (!stopped) {
        t_stop = std::chrono::high_resolution_clock::now();
      }
      stopped = true;
    }
    
    double getTotalTime() {
      return std::chrono::duration<double>(t_stop - t_start).count();
    }
    
    void print() {
      if (!stopped) { 
        std::cout << "currently running, can't print duration\n";
        return;
      }

      std::cout << "----- total Time: " << std::chrono::duration<double>(t_stop - t_start).count() << "s -----\n"; 
      if (t_checkpoints.size()>0) {
        std::cout << "checkpoint 0: " << 
          std::chrono::duration<double>(t_checkpoints[0] - t_start).count() << 
          's' << (comments[0].length() > 0 ? (", " + comments[0] + "\n") : "\n");
        for (int i=1; i<t_checkpoints.size(); ++i) {
          std::cout << "checkpoint " << i << ": " << 
            std::chrono::duration<double>(t_checkpoints[i] - t_checkpoints[i-1]).count() << 
            's' << (comments[i].length() > 0 ? (", " + comments[i] + "\n") : "\n");
        }
        std::cout << "after last checkpoint: " << std::chrono::duration<double>(t_stop - t_checkpoints[t_checkpoints.size()-1]).count() << "s\n";
      }
    }
  private:
    std::chrono::high_resolution_clock::time_point t_start, t_stop;
    std::vector<std::chrono::high_resolution_clock::time_point> t_checkpoints;
    std::vector<std::string> comments;
    bool stopped = true;
  };
  
  /**************************TIMEDLOOP******************************/
  class TimedLoop {
  public:
    //TimedLoop() = delete;
    TimedLoop(double ms) : counter(0), interval(ms) {
      t_start = std::chrono::high_resolution_clock::now();
    }
  
    const std::chrono::high_resolution_clock::time_point & get_start() {
      return t_start;
    }  
  
    void set_start(const std::chrono::high_resolution_clock::time_point & start) {
      t_start = start;
    }
  
    void set_interval(double ms) {
      interval = ms;
    }

	const double & get_interval() {
		return interval;
	}
    
    void set_start_now() {
      t_start = std::chrono::high_resolution_clock::now();
    }
  
    double get_timing(size_t pts) {
      return interval - 
        std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - 
        t_start).count() * 1000 + interval*pts;
    }
  
    void print_start() {
      std::time_t start_time_t = std::chrono::system_clock::to_time_t(t_start);
      std::cout << "start time: " << std::ctime(&start_time_t) << '\n';
      //std::cout << "weiß auch nicht...\n";
    }
  
    void add_offset(int offset_seconds) {
      std::lock_guard<std::mutex> guard(_mutex);
      t_start += std::chrono::seconds(offset_seconds);
    }
  
    float wait(size_t pts) {
      std::lock_guard<std::mutex> guard(_mutex);
      //std::cout << "arrived at:              " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 << '\n';
      //std::cout << "counter:                 " << counter << '\n';
      //std::cout << "should be released at:   " << (counter+1) * interval << '\n';
      double time_till_next = interval - std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 + interval*pts;
      //std::cout << "calculated waiting time with pts: " << time_till_next << '\n';
      if (time_till_next>0) {
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(time_till_next*1000)));
      }
  
      return time_till_next;
      //std::cout << "released at:             " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 << "\n --------------------- \n";
    }
  
    void wait() {
      std::lock_guard<std::mutex> guard(_mutex);
      //std::cout << "arrived at:              " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 << '\n';
      //std::cout << "counter:                 " << counter << '\n';
      //std::cout << "should be released at:   " << (counter+1) * interval << '\n';
      double time_till_next = interval - std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 + interval*counter;
      //std::cout << "calculated waiting time: " << time_till_next << '\n';
      if (time_till_next>0) {
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(time_till_next*1000)));
      }
      ++counter;
      //std::cout << "released at:             " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 << "\n --------------------- \n";
    }
  
    const std::chrono::high_resolution_clock::time_point & get_start_point() const {
      return t_start;
    }
  private:
    std::chrono::high_resolution_clock::time_point t_start;
    unsigned long long counter;
    double interval;//, time_till_next;
    std::mutex _mutex;
  };

  /******************************BUFFER*****************************/
  //single producer, single consumer FIFO queue
  template <typename T>
  class Buffer {
  private:
    std::vector<T> _data;
    std::vector<std::atomic<bool>> _occupied;
    int _size, _read_position, _write_position;

  public:
    //ctor
    Buffer(int size) : 
      _data(size), 
      _occupied(size), 
      _size(size), 
      _read_position(0), 
      _write_position(0) {}

    bool put(T elem) {
      if (_occupied[_write_position]) {
        //std::cout << "cant write at " << _write_position << "\n";
        return false;
      }
      //std::cout << "writing at " << _write_position << '\n';
      _data[_write_position] = std::move(elem);
      _occupied[_write_position] = true;
      _write_position = ++_write_position % _size;
      return true;
    }

    bool get(T & copy_to) {
      if (!_occupied[_read_position]) { 
        //std::cout << "cant read at"<< _read_position << '\n';
        return false;
      }
      //std::cout << "reading at " << _read_position << '\n';
      copy_to = std::move(_data[_read_position]);
      _occupied[_read_position] = false;
      _read_position = ++_read_position % _size;
      return true;
    }

    void clear() {
      for (int i=0; i<_data.size(); ++i) {
        _occupied[i] = false;
        _data[i] = T();
      }
      _read_position = 0;
      _write_position = 0;
    }
  };
}

#endif
