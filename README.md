# asyncio





​		如果您是一名服务器开发者，您有没有发现想使用的很多服务器框架很庞大，提供了太多的功能，依赖太多的库，构建起来很复杂，甚至要改变自己的使用习惯，已有的系统想用上这个框架需要修改太多的地方，甚至各种冲突。要么封装度不够，构建一个服务器还是有太多的工作要做。要么作者自己做了太多的改造，效果到底怎么样心里没底。如果您有这些顾虑，您可以考虑下asyncio。

​		如果您是一个新手，想做网游服务器，需要一些例子带您入门，这些例子需要足够精简，较少的依赖，构建起来要很容易，最好直接在这些例子上直接改改就能跑出一个正式的服务器，如果您还想长期干下去，想一条路能走到黑，Reactor模式几乎是您唯一正确的选择，您更加可以考虑下asyncio。

​		asyncio是一个易于使用的、轻量的、高效的服务器框架，基于Reactor模型，可用于迅速开发网游服务器、微服务、压测工具等，它不仅仅是一个网络框架，它抽象了网络，虽然目前使用了ASIO，但是将来却不一定，我希望将来你我都能升级它的各种组件。

​		一直以来想要一个用起来方便的网络组件，却一直没找到，只到我碰到了Python里面的asyncio，立刻被其优雅的接口和使用方式所折服，干脆自己写一个，所以就有了这个asyncio，连名字都一样，赫赫！

​		市面上已经有了很多种网络库，很多库也写得不错，比如muduo、evpp、brynet、libhv，为啥我还要再写一个？一方面我认为绝大多数网络库包括上面的这几个基本上都是为无状态服务设计的，对游戏这种有状态服务考虑的并不多，我坚持认为有状态服务的网络库要有一个SessionFactory去创建Session，网络接口应该是派生Session指针，并且Session的管理要在主线程中，还有好的解码器能让您的工作有积累，长期来看，这个方向会是时间的朋友。





## 特性

- C++11开发、使用方法类似Python内置的asyncio组件。
- 一个主线程+零到多个io线程，主线程处理逻辑，io线程进行报文收发。
- 主线程与io线程之间使用消息队列进行通信。
- 用户自定义Session、SessionFactory。
- 内置日志组件，支持用户对日志组件进行更换。
- 内置定时器，定时器依附在主线程或者io线程，实现毫秒级的定时触发。
- 内置多种解码器，这些解码器都支持较小的缓冲区接收较大的数据（时间换空间）。
- 跨平台，Windows、Linux、MacOS均能使用。
- 全头文件，用户无需编译即可使用。





## 依赖

- C++11
- asio





## 性能

- 测试环境：ping/pong测试，普通i7台式机，Linux
- 单线程版的服务器可以跑到16万QPS。
- 4个io线程+1个主线程的服务器可以跑到50万QPS。





## 示例

| 目录名称             | 说明                                                         |
| -------------------- | ------------------------------------------------------------ |
| 1.simple_client      | 一个简单的TCP客户端实现，用户自定义连接类MyConnection，自定义连接工厂MyConnectionFactory，可以实现连接、关闭、发送与接受数据功能。 |
| 2.simple_server      | 一个简单的TCP服务器实现，用户自定义会话类MySession，自定义会话工厂MySessionFactory，可以接受多个tcp客户端的连接，可以发送和接收数据、接受多个客户端连接功能。 |
| 3.chat_client        | 聊天客户端，如果网络断开，定时器会每隔3秒钟重连一次，只到连上为止，如果连上了，每隔2秒钟定时发送一条消息给服务器 |
| 4.chat_server        | 聊天服务器，会话实例使用递增的整数作为id，连接管理类可以对会话进行管理，客户端发送的每条消息都会广播给所有的客户端。客户端可以使用chat_client或者linux下的nc。 |
| 5.load_test_client   | 压测客户端，使用了解码器CodecLen，解决了黏包问题，收到的消息会原样反射给服务器，并对发送进行计数。 |
| 6.load_test_server   | 压测服务器，使用了解码器CodecLen，解决了黏包问题，收到的消息会原样反射给客户端，并对所有的消息进行计数。 |
| 7.gproto_test_client | 测试gproto协议的客户端，定时发送ping消息给服务器，并对服务器发过来的ping消息（心跳）回应pong消息。连接断开的时候能自动重连，能对消息进行计数。连续3次未收到对方的心跳会断开连接。 |
| 8.gproto_test_server | 测试gproto协议的服务器，定时发送ping消息给所有的客户端，并对客户端发送过来的ping消息回应pong消息。连续3次未收到对方的心跳会断开连接。 |
| 9.http_server        | HttpServer示例                                               |
|                      |                                                              |
|                      |                                                              |
|                      |                                                              |
|                      |                                                              |
|                      |                                                              |
|                      |                                                              |
|                      |                                                              |
|                      |                                                              |
|                      |                                                              |
|                      |                                                              |



## 计划

| 编号 | 内容                                                         | 状态                 |
| ---- | ------------------------------------------------------------ | -------------------- |
| 1    | 参考一下 cpp_redis的封装方式 https://github.com/cpp-redis/cpp_redis |                      |
| 2    | 添加对HttpServer的支持                                       | 20210831 Will 已完成 |
| 3    | 添加支持CronTab表达式的定时器                                |                      |
| 4    | 用户态协议的支持                                             |                      |
| 5    | 使用多种网络库按照asyncio的接口实现一遍，对比一下性能        |                      |
| 6    | 使用travis进行持续集成，防止出现各个平台编译不过的问题       |                      |
| 7    | 做一个RPC解码器，客户端和服务器示例                          |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |
|      |                                                              |                      |



## 一些设计上的观点

这个框架初一看可能和muduo、evpp很像，但是又不完全一样，我是有一些改动的，比如：

1. 解码器是有状态的，这是为了让最大报文长度不依赖于接收缓冲区，解码器和TcpConn应该一一对应。在muduo中，解码器没有状态，所以可以和TcpConn之间一对多。最大报文长度不依赖接收缓冲区有个最大的好处，就是可以接收长报文，很多网络框架只能接受64K的报文，如果你要传一张10M的图片是不是很不方便？如果你把缓冲区扩充到10M，服务器的内存开销又大了，两难对不对？
2. 网络的io线程、定时器线程不在主线程中，所以这些子线程需要通过主线程的消息队列发送事件。所以有定时器消息、新连接创建、连接结果，连接关闭，网络消息这五种操作是需要使用消息队列的，如果不使用消息队列，会有多线程操作的风险。
3. TcpServer的连接列表做在了主线程中，我认为主线程更需要这个列表，而且主线程可能还需要有各种方式的查询（比如按照SessionID、用户名等），muduo和evpp是在io线程中的，这点不一样。





## 安装与使用

依赖库：asio，建议使用vcpkg对第三方库进行管理。编译器需要支持C++11。





## 致谢

向我参考过的框架，还有一些讨论过的朋友（如果您愿意，您的名字会列在下面）致谢！当然还有伟大的asio和Python。

| 编号 | 名称             | 地址                                         |
| ---- | ---------------- | -------------------------------------------- |
| 1    | 奇虎的网络库evpp | https://github.com/Qihoo360/evpp             |
| 2    | muduo            | https://github.com/chenshuo/muduo            |
| 3    | asio2            | https://github.com/zhllxt/asio2              |
| 4    | brynet           | https://github.com/IronsDu/brynet            |
| 5    | redis-cpp17      | https://github.com/danielshaving/redis-cpp17 |
| 6    | libhv            | https://github.com/ithewei/libhv.git         |
|      |                  |                                              |
|      |                  |                                              |
|      |                  |                                              |
|      |                  |                                              |





## 联系我们

群名称：后端技术爱好者交流群

群号码：915372354

点击链接加入群聊【后端技术爱好者交流群】：https://jq.qq.com/?_wv=1027&k=GXi07Vh6



Will

