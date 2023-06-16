#ifndef V2_CORE_DB_H_
#define V2_CORE_DB_H_

#include "objs.h"
#include <vector>
#include <core/glass_object.h>

namespace SQLite
{
class Database;
}

//注意了，不支持多线程使用
//操作中都没有对db加锁
class DB
{
public:
  static DB* get();
  ~DB();

  bool open();
  void close();

  void initTables();

  void dropTables();
  void dropTables2();

public:
  //查询出所有玻璃类型
  std::vector<GlassConfig> queryAllGlassConfigs();

  //返回所有玻璃类型的名称
  std::vector<std::string> queryAllGlassConfigNames();

  //判断某个型号是否存在
  bool isGlassConfigExist(const std::string& name)
  {
    GlassConfig config;
    return queryGlassConfigByName(name, config);
  }

  //根据名称查询
  //存在则返回true,否则返回false
  bool queryGlassConfigByName(const std::string& name, GlassConfig& re);

  //保存类型，name已存在则更新，不存在则添加
  bool saveGlassConfig(const GlassConfig& config);

public:
  //查询出所有批号
  std::vector<Batch> queryAllBatches();

  //判断某个批号是否存在
  bool isBatchExist(const std::string& batch_id)
  {
    Batch re;
    return queryBatchById(batch_id, re);
  }

  //根据名称查询
  //存在则返回true,否则返回false
  bool queryBatchById(const std::string& batch_id, Batch& re);

  //添加
  bool addBatch(const Batch& b);

public:
  //保存缺陷记录
  bool addFlaw(const FlawRecord& re);

public:
  //增加一张玻璃到统计
  bool addGlassToStatistics(const std::string& batch_id, std::shared_ptr<GlassObject> glass);

  //查询一个批次统计
  bool queryBatchStatisticsRecord(const std::string& batch_id, BatchStatisticsRecord& re);

  //查询一个总检统计
  bool queryAllStatisticsRecord(AllStatisticsRecord& re);

private:
  DB();
  std::shared_ptr<SQLite::Database> _db;

  bool _dropTable(const std::string& tb);

  bool _createGlassConfigTable();
  bool _createBatchTable();
  bool _createFlawTable();
  bool _createStatisticsTable();
  bool _createTable(const std::string& tb, const std::string& sql);

  bool _queryBatchStatisticsRecord(const std::string& batch_id, BatchStatisticsRecord& re);
  bool _queryAllStatisticsRecord(AllStatisticsRecord& re);
  AllStatisticsRecord _all_cache;
  bool _all_cached;
  std::map<std::string, BatchStatisticsRecord> _batch_cache;
};

#endif //V2_CORE_DB_H_
