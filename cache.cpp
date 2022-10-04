#include <bits/stdc++.h>

using namespace std;
typedef long double ld;

enum policyType { FIFO = 0, LRU = 1 };
enum OpType { READ = 0, WRITE = 1, READ_INSTRUCTION = 2 };

class Cache {
 private:
  bool dirty = false;
  uint32_t tag = 0;

 public:
  Cache(bool _v, uint32_t _tag) : dirty(_v), tag(_tag) {}
  const uint32_t getTag() { return tag; }
  void setCache(const bool &dirty, const uint32_t &tag) {
    this->dirty = dirty;
    this->tag = tag;
  }
  bool operator==(const uint32_t &a) const { return (tag == a); }
  bool operator==(const bool &a) const { return (dirty == a); }
};

class CacheController {
 private:
  uint32_t indexNum;
  uint32_t dataNum;
  uint32_t blockSize;
  policyType policy;  // policy type
  vector<vector<Cache>> cache;

  // for statistics
  uint32_t cacheHit = 0;
  uint32_t cacheMiss = 0;
  uint32_t readData = 0;
  uint32_t writeData = 0;
  uint32_t bytetoMemory = 0;
  uint32_t byteFromMemory = 0;

 public:
  CacheController(const uint32_t &CacheSize, const uint32_t &blockSize,
                  const uint32_t &associativity, const string &policyString) {
    this->blockSize = blockSize;
    if (associativity == 0x1) {  // direct-mapped
      indexNum = CacheSize * 1024 / blockSize;
      dataNum = 1;
    } else if (associativity == 0xf) {  // fully associative
      indexNum = 1;
      dataNum = CacheSize * 1024 / blockSize;
    } else {  // n-way cache
      indexNum = CacheSize * 1024 / blockSize / associativity;
      dataNum = associativity;
    }

    if (policyString.compare("FIFO") == 0)
      policy = FIFO;
    else if (policyString.compare("LRU") == 0)
      policy = LRU;
    else {
      cerr << "Policy Error" << endl;
      exit(-1);
    }
    
    cache.resize(indexNum);
    for (auto &it : cache) {
      it.resize(dataNum, Cache(false, 0));
    }
  }

  void operation(const uint32_t &op, const uint32_t &address) {
    uint32_t tag = address / blockSize;
    uint32_t idx = address / blockSize % indexNum;
    auto it = find(cache[idx].begin(), cache[idx].end(), tag);

    if (op == READ) {
      readData++;
    } else if (op == WRITE) {
      writeData++;
    } else if (op == READ_INSTRUCTION) {
    } else {
      perror("Error Op code");
      exit(-1);
    }

    if (it != cache[idx].end()) {  // cache hit
      cacheHit++;
      if (op == WRITE) {
        it->setCache(true, tag);
      }
      if (policy == LRU) {
        Cache tmp = *it;
        cache[idx].erase(it);
        cache[idx].insert(cache[idx].begin(), tmp);
      }
    } else {  // cache miss
      cacheMiss++;
      byteFromMemory += blockSize;
      int size = cache[idx].size();
      cache[idx].insert(cache[idx].begin(), Cache((op == WRITE), tag));
      size = cache[idx].size();
      size = cache[idx].size();
    }

    if (cache[idx].size() > dataNum) {
      if (cache[idx].back() == true)
        bytetoMemory += blockSize;  // need update Mem
      cache[idx].pop_back();
    }
  }

  // clean cahce and write dirty data to mem
  void forceToMem() {
    for (auto it : cache) {
      while (it.empty() == false) {
        if (it.back() == true) bytetoMemory += blockSize;
        it.pop_back();
      }
    }
  }

  void printResult() {
    cout << "Demand fetch = " << cacheHit + cacheMiss << endl
         << "Cache hit = " << cacheHit << endl
         << "Cache miss = " << cacheMiss << endl
         << "Miss rate = " << fixed << setprecision(4) << (ld)cacheMiss / (ld)(cacheHit + cacheMiss) << endl
         << "Read data = " << readData << endl
         << "Write data = " << writeData << endl
         << "Bytes from Memory = " << byteFromMemory << endl
         << "Bytes to memory = " << bytetoMemory << endl;
  }
};

int main(int argc, char *argv[]) {
  if (argc != 6) {
    cerr << "args error";
    exit(-1);
  }

  CacheController cacheController(atoi(argv[1]), atoi(argv[2]),
                                  strtol(argv[3], NULL, 16), argv[4]);

  fstream file(argv[5], ios::in);
  uint32_t op;
  uint32_t address;

  while (file >> op) {
    file >> hex >> address;
    if (address == 0xffffffff) continue;
    cacheController.operation(op, address);
  }
  
  cacheController.forceToMem();

  cout << "Input file = " << argv[5] << endl;
  cacheController.printResult();

  file.close();
}