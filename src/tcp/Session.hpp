
#ifndef SESSION_HPP
#define SESSION_HPP

#include <onions-common/tcp/HandleAlloc.hpp>
#include <json/json.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>

typedef std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;
typedef boost::array<char, 1024> Buffer;

class Session : public boost::enable_shared_from_this<Session>
{
 public:
  Session(const SocketPtr&, int);
  ~Session();
  int getID() const;

  void asyncRead();
  void asyncWrite(const std::string&, const std::string&);
  void asyncWrite(const Json::Value&);

 private:
  void processMessage(const std::string&);
  bool respond(const Json::Value&, Json::Value&);
  Json::Value respondToPutRecord(const Json::Value&) const;
  Json::Value respondToDomainQuery(const Json::Value&) const;
  Json::Value respondToGetMerkleSubtree(const Json::Value&) const;

  void processRead(const boost::system::error_code&, size_t);
  void processWrite(const boost::system::error_code&,
                    const std::shared_ptr<Buffer>&);
  void asyncWrite(const std::string&);

  SocketPtr socket_;
  Buffer inChunk_;
  std::string inBuffer_;
  HandleAlloc allocator_;
  int id_;
};

#endif
