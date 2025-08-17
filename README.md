# elog

elog 是一个性能非常高效的 C++ 异步日志库，支持五种日志级别的输出，输出百万行日志所需时间不到1秒。

## 目录

- [缓冲区设计](#缓冲区设计)
- [智能指针的妙用](#智能指针的妙用)
- [多线程的引入](#多线程的引入)
- [三层级处理](#三层级处理)
  - [AsyncLogging](#asynclogging)
  - [LogFile](#logfile)
  - [FileAppender](#fileappender)

## 缓冲区设计

- 采用双缓冲机制，日志写入到缓存buffer中，如果缓存满了则转移一个容器buffers中，等待日志落盘，避免日志写入和落盘产生冲突
- 使用swap函数将buffers中的数据转移到辅助容器buffersToWrite中，后续操作在buffersToWrite中进行，大幅缩小临界区
- 全程使用移动语义进行缓存的转移，避免数据拷贝
- 复用已申请的内存，尽可能减少重复申请
- 使用ThreadLocal创建线程本地变量，减少系统调用，提高性能

## 智能指针的妙用

- 减少拷贝次数
- 减少多线程的加锁临界区

## 多线程的引入

- 采用生产者消费者模型
- 定时消费，避免饥饿
- 生产过快，瞬时数据量过大，丢弃并通知前端

## 三层级设计

为了实现高内聚低耦合，elog 采用三层级处理框架：`AsyncLogging -> LogFile -> FileAppender`，降低了模块间的依赖性，增强了可扩展性。

### AsyncLogging

- 当前端缓冲区满或超时，则进行一次消费。
- 消费过程如下：
  1. push 并更新前端缓存块（cur 和 nextBuffer）。
  2. 交换用于消费队列的内存（大幅减小临界区）。
  3. 判断队列数据是否过量，过量则丢弃，并通知前端。
  4. 将队列中的缓存块调用 `LogFile` 的 append 方法写入。
  5. 保留两个数据，进行数据重复利用的判断。
  6. 调用 `LogFile` 的 flush 刷新缓冲区，然后 clear，进行下一轮消费的等待。

### LogFile

- 当 `FileAppender` 写入数据超出规定，进行 check 时距离上次超过一天则进行 rollFile。
- 进行 check 时，距离上次超过 `flushInterval`，则进行 flush。
- rollFile 过程如下：
  1. 判断距离上次 rollFile 是否过去 1 秒钟以上。
  2. 根据年月日时分秒以及进程 id 和用户名得到唯一的用户名。
  3. 重新创建新的 `FileAppender`（切换独占指针管理的内存）。

### FileAppender

- 手动设置 FILE* 的缓冲区大小。
- 封装 flush 和 write 操作：
  1. flush：就是对 flush 的调用。
  2. write：调用无锁版本的 write -> fwrite_unlocked。
- 保证 append 操作的可靠性：
  1. 通过 while 循环保证每次写入是正确的，如果写入发生错误则抛出异常。
  2. 如果本次写入内容过多，通过 while 循环也能确保最终能完全写入。
  3. 记录写入的字节数，方便上层调用判断是否 rollFile。
