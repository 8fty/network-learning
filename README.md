# network-learning

Linux 下 C++ 网络编程学习代码库：从最基础的 socket 封装，到多进程 / 多线程 / 线程池并发模型、自定义应用层协议，再到一个能返回静态资源的 HTTP 服务器。

每个子目录都是一个可以独立编译、独立运行的小项目，按"先跑通、再演进"的顺序排列，保留了演进过程中的多种实现（多进程 / 多线程 / 线程池版本在代码中以注释形式保留），方便对照学习。

## 目录结构

| 目录 | 内容 | 关键技术点 |
| --- | --- | --- |
| `TCP/` | TCP echo 服务器 + 客户端 | `socket` / `bind` / `listen` / `accept`、多进程、多线程、**线程池**、单例、日志、守护进程 |
| `tcp_protocol/` | TCP 计算器（自定义应用层协议） | **报文定长 + 序列化 / 反序列化**、`jsoncpp`、协议与业务解耦 |
| `UDP/` | UDP 群聊服务器 + 客户端 | `recvfrom` / `sendto`、在线用户管理、**广播**、回调式业务处理 |
| `UDPTest/` | UDP echo 服务器（最小示例） | UDP 收发最小闭环 |
| `http_7_3/` | HTTP 静态资源服务器 | HTTP 请求解析、`stat` 获取资源大小、`Content-Type`、`wwwroot` 静态站点、404 处理 |

## 环境要求

* **Linux**（或 WSL2 / Linux 虚拟机）——代码直接使用 `sys/socket.h`、`unistd.h`、`pthread.h`、`fork()` 等 POSIX 接口，**无法在原生 Windows 上编译**
* g++ 支持 C++11
* `tcp_protocol/` 额外依赖 **jsoncpp**：

```bash
# Ubuntu / Debian
sudo apt install -y g++ make libjsoncpp-dev
```

## 快速开始

各目录均自带 `makefile`，进入目录后 `make` 即可。

### 1. TCP echo 服务器（`TCP/`）

```bash
cd TCP
make                     # 生成 tcpserver / tcpclient

./tcpserver 8080         # 注意：服务器会调用 daemonSelf() 转为守护进程
./tcpclient 127.0.0.1 8080
```

客户端输入任意内容，服务器返回 `内容 + server[echo]`。

> 调试提示：`tcpServer.cc` 中的 `daemonSelf()` 会把标准输入输出重定向到 `/dev/null`，转为守护进程后终端看不到日志。学习调试时把这一行注释掉即可，后台运行时用 `ps aux | grep tcpserver` 找到进程再 `kill` 掉。

### 2. TCP 计算器 · 自定义协议（`tcp_protocol/`）

```bash
cd tcp_protocol
make                     # 生成 calserver / calclient，需已安装 libjsoncpp-dev

./calserver 8081
./calclient 127.0.0.1 8081
# 输入形如 1+1、10*3、7%2、6/0 的表达式
```

支持 `+ - * / %`；除数为 0 返回 `DIV_ZERO`，模数为 0 返回 `MOD_ZERO`，未知运算符返回 `OP_ERROR`。

### 3. UDP 群聊（`UDP/`）

```bash
cd UDP
make                     # 生成 udpServer / udpClient

./udpServer 8082
./udpClient 127.0.0.1 8082    # 可开多个客户端
```

协议为纯文本命令：

* 客户端输入 `online` —— 注册到在线列表
* 客户端输入 `offline` —— 从在线列表注销
* 未登录时发消息，服务器回 `你没有登录,请运行online`
* 登录后发消息，服务器会把 `ip-port#消息` **广播** 给所有在线用户

> `udpServer.cc` 中保留了三套业务回调：`handlerMessage`（字典查询，需 `dict.txt`）、`execCommand`（执行 shell 命令）、`routeMessage`（群聊，当前启用）。切换时只需修改 `main` 中构造 `udpServer` 时传入的函数。

### 4. UDP echo（`UDPTest/`）

```bash
cd UDPTest
make                     # 生成 udpServer
./udpServer 8083         # 可直接用 UDP/ 目录的 udpClient 连接测试
```

### 5. HTTP 静态服务器（`http_7_3/`）

```bash
cd http_7_3
make                     # 生成 httpserver
./httpserver 8084        # 必须在 http_7_3 目录下运行，wwwroot 是相对路径

curl -v http://127.0.0.1:8084/            # 首页
curl -v http://127.0.0.1:8084/test/a.html
curl -v http://127.0.0.1:8084/notexist    # 404 页面
```

静态站点位于 `http_7_3/wwwroot/`：`index.html` 为首页（含图片与子页面链接），`404.html` 为资源不存在时的兜底页。服务器会按请求 URL 拼接 `./wwwroot` 作为资源路径，用 `stat()` 取文件大小作为 `Content-Length`，并按后缀设置 `Content-Type`。

## 实现要点

### 并发模型的演进（`TCP/`）

同一个服务器依次实现了四种处理连接的方式，代码中保留了对比：

1. **单进程循环**：`serviceio(sock)` 直接串行处理，一次只能服务一个客户端
2. **多进程**：`fork()` 后子进程处理业务，父进程 `waitpid` 回收；子进程二次 `fork` 让孙子进程处理，父进程立即返回，避免僵尸进程堆积
3. **多线程**：每个连接创建一个 `pthread_detach` 的线程
4. **线程池**：`ThreadPool<Task>` 单例（双检查锁），`pthread_mutex` + `pthread_cond` 实现任务队列的生产者—消费者模型，主线程 `accept` 后把任务 `push` 进队列

### 自定义应用层协议（`tcp_protocol/`）

解决 TCP 粘包 / 半包问题的做法是**给每个报文加长度前缀**：

```
"正文长度"\r\n"正文"\r\n
```

收发流程完全解耦：

```
recvPackage()  →  deLength()  →  Request::Deserialize()   ← 网络层读满一个完整报文
                                     ↓
                              业务函数 cal(req, resp)      ← 只关心业务，不碰 IO
                                     ↓
enLength()     ←  serialize()   ←  Response
```

`Protocol.hpp` 中通过 `#define MYSELF` 宏提供了两种序列化方案，默认使用 `jsoncpp`：

* **未定义 `MYSELF`**：JSON 序列化（`{"first":1,"second":1,"oper":43}`）
* **定义 `MYSELF`**：自定义文本协议，如 `1 + 1` / `exitcode result`

服务端用一个 `std::function<bool(const Request&, Response&)>` 类型的回调注册业务逻辑，做到协议层与业务层分离，业务函数 `cal()` 只负责计算并填充响应。

### UDP 群聊的在线管理（`UDP/`）

`OnlineUser` 以 `ip-port` 为 key 维护 `unordered_map`，提供 `addUser` / `delUser` / `isOnline` / `broadcast`。由于 UDP 无连接，服务器只靠每个数据包携带的 `sockaddr_in` 识别发送者，广播时再逐个 `sendto`。

## 已知问题与改进方向

这些是学习过程中的留待完善之处，也正是继续深入的方向：

* **`TCP/ThreadPool.hpp`**：`pop()` 在队列为空时直接取 `front()`，建议在持锁状态下判断非空后再取，并改用条件变量唤醒替代轮询判断
* **`tcp_protocol/Protocol.hpp`**：`recvPackage()` 中 `find(LINE_SEP)` 未判断 `npos`，且使用 `inbuffer += buffer`（应为 `append(buffer, n)`），报文不完整时存在越界风险
* **`http_7_3/`**：只读取一次 `recv` 不保证读到完整请求头；`Content-Type` 中 `jpg` 应使用标准的 `image/jpeg`；`readFile` 用 `resize` 后的缓冲区直接读写 `c_str()`，建议改用 `std::vector<char>`
* **`tcp_protocol/calClient.hpp`**：`ParseLine()` 仅支持"整数 运算符 整数"，不支持负数与多位数带空格的输入
* **`UDP/`**：无心跳与超时机制，客户端异常退出后仍会残留在在线列表中
* 全部项目均为**阻塞 IO + 多进程/多线程**模型，后续可演进为 `epoll` 的 IO 多路复用 + 非阻塞 + Reactor 模式

## 说明

本仓库为个人网络编程学习记录，代码以"理解原理"为目标，工程健壮性有限，请勿直接用于生产环境。
