// 单例是创建型的设计模式
// 通过单例模式的方法创建的类在当前进程中只有一个实例

// 应用场景
// * 配置管理\日志记录\线程池\链接池\内存池\对象池\消息队列

// 实现步骤
//  1. 将类的构造方法定义为私有方法
//  2. 定义一个私有的类的静态实例
//  3. 提供一个共有的获取实例的静态方法
//  这个设计没有考虑线程安全, 所以使用的时候要
// 在单线程中创建, 在多线程中使用

#ifndef Singleton_H__
#define Singleton_H__

template <typename T>
class Singleton {
 private:
  static T* m_instance;

  Singleton() {}
  Singleton(const Singleton<T>&);
  Singleton<T>& operator=(const Singleton<T>&);
  ~Singleton() {}

 public:
  static T* instance() {
    if (m_instance == nullptr) m_instance = new T();
    return m_instance;
  }
};

template <typename T>
T* Singleton<T>::m_instance = nullptr;

#endif