# User Guide
设计思路及用户引导
## Threads

服务器线程由监听线程FListenThread 连接池线程 FConnectThreadPoolThread 和 连接线程FConnectThread 组成

启动服务器监听线程
```
ServerThread = FListenThread::Create(Port, PoolTimespan);

// block kill
ServerThread->KillThread();
delete ServerThread;
ServerThread = nullptr;

// unblock Kill
ServerThread->Stop
```

## Protocol 协议

### TSNetPacket\<FStaticNetBodyBase_Child\> 网络包
一个网络包由head body foot三部分组成

```
FSNetBufferHead Head;
TSNetBufferWrapper\<FStaticNetBodyBase_Child\> Body;
FSNetBufferFoot Foot;
```

FStaticNetBodyBase_Child 即是自定义的buffer类

 1. 定义一个网络包协议

 参考 TestPacket.h

```
 //只需声明一个继承自 FStaticNetBodyBase 的 netbody类
 #pragma pack(pop)
 #pragma pack(push, 8)
 struct FNetBodyType : public FStaticNetBodyBase
 {
   // write struct data here
   // ...
   // override memsize
   virtual int32 MemSize() override
   {
     return sizeof(FStaticNetBodyBase_Child);
   }
 };
 #pragma pack(pop)
```

  2. 向工厂类FProtocolFactory注册网络包

```
int32 UID = 1005;
FProtocolFactory::Get()->RegisterProtocolDeserialize(UID,
	, std::bind(&TServoProtocol < FNetBodyType, ESPMode::ThreadSafe, SPPMode::Queue>::OnReceivedPacket
	, std::placeholders::_1
	, std::placeholders::_2
	, std::placeholders::_3
	, std::placeholders::_4
);
```
或使用宏
```
int32 UID = 1005;
__REG_PROTOCOL_NETBODY_THREADSAFE_QUEUE(uid, FNetBodyType)
```

  3. 调用工厂类解码网络包

查询网络包注册状态
```
  FProtocolFactory::Get()->IsProtocolRegister(PacketHead.uid)
```
执行已注册解包函数
```
FProtocolFactory::Get()->CallProtocolDeserializeWithoutCheck(PacketHead, bufferPtr, bufferLen, RecivedBytesRead);
```
局部代码样例：安全调用, 参考ConnectThread.cpp
```
if (PacketHead.IsSerializedPacket() && FProtocolFactory::Get()->IsProtocolRegister(PacketHead.uid))
{
  ProtocolFactory->CallProtocolDeserializeWithoutCheck(PacketHead, ReceivedData.GetData() + TotalBytesRead, ReceivedData.Num() - TotalBytesRead, RecivedBytesRead);
  TotalBytesRead += RecivedBytesRead;
}
```

### 小结
1. 定义一个网络包协议, 继承FStaticNetBodyBase
2. 向工厂类FProtocolFactory注册网络包
3. 调用工厂类解码网络包

## F.A.Q

### How to make a new netpacket?

### How to create a tcp server?

### How to create a tcp client?
