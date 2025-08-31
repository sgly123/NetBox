# 三国杀联机版

基于NetBox框架实现的多人在线三国杀游戏。

## 功能特点

1. **多人对战**
   - 支持2-8人游戏
   - 房间系统
   - 观战模式

2. **游戏机制**
   - 角色系统
   - 卡牌系统
   - 技能系统
   - 回合制战斗

3. **网络特性**
   - 实时同步
   - 断线重连
   - 游戏存档
   - 观战直播

## 实现要点

### 1. 协议设计

```cpp
// 游戏协议
struct GameProtocol : public ProtocolBase {
    // 消息类型
    enum class MsgType {
        ROOM_CREATE,    // 创建房间
        ROOM_JOIN,      // 加入房间
        GAME_START,     // 游戏开始
        CARD_USE,       // 使用卡牌
        SKILL_USE,      // 使用技能
        TURN_END,       // 回合结束
        GAME_SYNC      // 游戏同步
    };
    
    // 协议头
    struct Header {
        uint32_t magic;     // 魔数
        uint16_t version;   // 版本号
        uint16_t msgType;   // 消息类型
        uint32_t sequence;  // 序列号
        uint32_t length;    // 数据长度
    };
};
```

### 2. 游戏服务器

```cpp
class SanguoServer : public ApplicationServer {
public:
    SanguoServer(const std::string& ip, int port);
    
protected:
    // 初始化协议
    void initializeProtocolRouter() override;
    
    // 处理游戏消息
    void handleGameMessage(const GameMessage& msg);
    
    // 房间管理
    Room* createRoom(const std::string& name);
    bool joinRoom(int roomId, Player* player);
    
private:
    std::unordered_map<int, std::unique_ptr<Room>> m_rooms;
    std::unordered_map<int, Player*> m_players;
};
```

### 3. 游戏逻辑

```cpp
// 角色系统
class Hero {
public:
    virtual void useSkill(const SkillContext& ctx) = 0;
    virtual bool checkSkillCondition(const SkillContext& ctx) = 0;
    
protected:
    int m_hp;
    std::vector<Skill*> m_skills;
};

// 卡牌系统
class Card {
public:
    virtual void onUse(Player* from, Player* to) = 0;
    virtual bool canUse(Player* from, Player* to) = 0;
    
protected:
    CardType m_type;
    CardSuit m_suit;
    int m_point;
};

// 回合系统
class TurnSystem {
public:
    void nextTurn();
    void addPhase(Phase* phase);
    bool checkPhaseCondition(Phase* phase);
    
private:
    std::vector<Phase*> m_phases;
    Player* m_currentPlayer;
};
```

## 使用说明

### 1. 编译和运行

```bash
# 编译
cd build
cmake ..
make sanguo_server

# 运行服务器
./sanguo_server

# 运行客户端
./sanguo_client
```

### 2. 配置说明

编辑 `config/sanguo.json`:
```json
{
    "server": {
        "host": "127.0.0.1",
        "port": 8888,
        "max_rooms": 100,
        "max_players_per_room": 8
    },
    "game": {
        "init_cards": 4,
        "max_hp": 4,
        "turn_timeout": 60
    }
}
```

### 3. 客户端命令

```bash
# 创建房间
/create <room_name>

# 加入房间
/join <room_id>

# 准备
/ready

# 使用卡牌
/use <card_id> <target_player>

# 使用技能
/skill <skill_id> <target_player>

# 结束回合
/end
```

## 测试方法

### 1. 单元测试

```bash
# 运行所有测试
./test_sanguo

# 运行特定测试
./test_sanguo --gtest_filter=CardTest.*
```

### 2. 集成测试

```bash
# 模拟多人游戏
python test_multi_player.py

# 压力测试
python test_stress.py --rooms 50 --players 400
```

### 3. 测试用例

1. **基础功能测试**
   - 房间创建和加入
   - 角色选择
   - 卡牌使用
   - 技能触发
   - 回合流转

2. **网络测试**
   - 断线重连
   - 游戏同步
   - 高延迟处理
   - 丢包处理

3. **并发测试**
   - 多房间并发
   - 大量玩家同时操作
   - 观战玩家并发

## 扩展建议

1. **新角色开发**
   - 继承Hero类
   - 实现技能接口
   - 添加角色配置
   - 编写单元测试

2. **新卡牌开发**
   - 继承Card类
   - 实现使用效果
   - 添加卡牌配置
   - 编写单元测试

3. **新玩法开发**
   - 修改回合系统
   - 添加新的游戏模式
   - 扩展房间配置
   - 更新协议定义

## 性能优化

1. **网络优化**
   - 使用二进制协议
   - 实现消息压缩
   - 优化同步策略
   - 实现预测回滚

2. **并发优化**
   - 房间分片
   - 消息队列
   - 状态缓存
   - 延迟更新

3. **资源优化**
   - 内存池
   - 对象复用
   - 配置缓存
   - 延迟加载

## 调试技巧

1. **日志查看**
```bash
# 查看游戏日志
tail -f logs/sanguo.log

# 查看错误日志
grep "ERROR" logs/sanguo.log
```

2. **状态查看**
```bash
# 查看房间状态
./sanguo_admin --rooms

# 查看玩家状态
./sanguo_admin --players
```

3. **性能分析**
```bash
# 性能分析
./sanguo_profile

# 内存分析
valgrind --tool=massif ./sanguo_server
```

## 常见问题

1. **游戏卡顿**
   - 检查网络延迟
   - 查看服务器负载
   - 优化同步策略
   - 减少消息频率

2. **游戏崩溃**
   - 检查错误日志
   - 分析崩溃堆栈
   - 添加异常处理
   - 实现自动恢复

3. **数据不同步**
   - 检查序列号
   - 验证状态一致性
   - 实现状态校验
   - 强制同步机制 