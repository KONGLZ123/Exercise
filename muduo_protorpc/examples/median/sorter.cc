#include <stdlib.h>
#include <muduo/net/InetAddress.h>

bool debug = false;

namespace median
{

class SorterImpl : public Sorter
{
 public:
  SorterImpl() 
  {
    xsubi_[0] = static_cast<unsigned short>(getpid());
    xsubi_[1] = static_cast<unsigned short>(gethostid());
    xsubi_[2] = static_cast<unsigned short>(Timestamp.now());
    doGenerate(100, 0, 30);
  }

 private:
  void doGenerate(int count, int min, int max) {
    data_.clear();
    for (int i = 0; i < count; ++i) {
      int64_t range = max - min;
      int64_t value = min;
      if (range > 1)
	value += nrand48(xsubi_) % range;
      data_.push_back(value);
      std::sort(data_.begin(), data_.end());
      if (debug) {
        std::copy(data_.begin(), data_.end(), std::ostream_iterator<int64_t>(std::cout, " "));
	std::cout << std::endl;
      }
    }
  }
  std::vector<int64_t> data_;
  unsigned short xsubi_[3]; 
};

}

int main(int argc, char *argv[])
{
  int port = argc > 1 ? aoti(argv[1]) : 5555;
  debug = argv > 2;
  InetAddress listenAddr(static_cast<uint16_t>(port));
  median::SorterImpl impl;
}



