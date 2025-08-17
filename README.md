# elog

elog 是一个性能非常高效的 C++ 异步日志库，支持五种日志级别的输出，输出百万行日志所需时间不到1秒。

## 目录

- [缓冲区设计](#缓冲区设计)
- [多线程的引入](#多线程的引入)
- [三层级设计](#三层级设计)
  - [AsyncLogging](#asynclogging)
  - [LogFile](#logfile)
  - [FileAppender](#fileappender)

## 缓冲区设计

- 采用双缓冲机制，日志写入到前端缓存块 curBuffer 中，如果缓存块满了则转移一个容器 buffers 中，等待日志落盘，避免日志写入和落盘产生冲突。
- 使用 swap 函数将 buffers 中的数据转移到辅助容器 buffersToWrite 中，后续操作在 buffersToWrite 中进行，大幅缩小临界区。
- 全程使用移动语义进行缓存块的转移，避免数据拷贝。
- 复用已申请的内存，尽可能减少重复申请。
- 使用 ThreadLocal 线程本地变量缓存系统调用，减少调用次数，提高性能。

## 多线程的引入

- 采用生产者消费者模型。
- 定时消费，避免饥饿。
- 生产过快，瞬时数据量过大则丢弃（保证效率），并通知前端。

## 三层级设计

为了实现高内聚低耦合，elog 采用三层级处理框架：`AsyncLogging -> LogFile -> FileAppender`，降低了模块间的依赖性，增强了可扩展性。

### AsyncLogging

- 当前端缓冲区满或超时，则进行一次消费。
- 消费过程如下：
  1) push 并更新前端缓存块（curBuffer 和备用缓存块 nextBuffer）。
  2) 交换消费队列的内存（大幅减小临界区）。
  3) 判断队列数据是否过量，过量则丢弃，并通知前端。
  4) 将队列中的缓存块调用 `LogFile` 的 append 方法写入（最终调用 `FileAppender` 的 append）。
  5) 将 buffersToWrite 清空前保留两个缓存块，转移给 curBuffer 和 nextBuffer 进行内存的复用。
  6) 调用 `LogFile` 的 flush 刷新缓冲区（最终调用 `FileAppender` 的 flush），然后清空 buffersToWrite，进行下一轮消费的等待。

### LogFile

- 当 `FileAppender` 写入的数据超出规定大小，或者进行 check 时发现与上次 check 不在同一天内，则进行 rollFile。
- 进行 check 时，如果发现距离上次 check 超过 `flushInterval`，则进行 flush。
- rollFile 过程如下：
  1. 判断距离上次 rollFile 是否过去 1 秒以上。
  2. 根据年月日时分秒以及进程 id 和用户名得到唯一的文件名。
  3. 创建新的 `FileAppender`（创建一个新的文件用于写入）。

### FileAppender

- 封装 flush 和 write 操作：
  1. flush：对 fflush 的调用。
  2. write：调用无锁版本的 fwrite_unlocked。
- 保证 append 操作的可靠性：
  1. 通过 while 循环保证每次写入是正确的，如果发生写入错误则抛出异常。
  2. 如果本次写入内容过多，通过 while 循环也能确保最终能完全写入。
  3. 记录写入的字节数，方便 `LogFile` 调用判断是否需要 rollFile。
