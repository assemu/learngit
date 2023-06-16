#include "db.h"
#include <core/util/log.h>
#include <core/util/str_util.h>
#include <core/util/time_util.h>
#include <core/app_config.h>
#include <core/db/sqlite/SQLiteCpp.h>

DB* DB::get()
{
  static DB* _inst = NULL;
  if(_inst == NULL)
    _inst = new DB();
  return _inst;
}

DB::~DB()
{
}

DB::DB()
{
  _all_cached = false;
}

bool DB::open()
{
  auto db_path = str_utf8_to_system(AppConfig::get()->getAppHomeDir() + "/main.db").c_str();
  try {
    _db = std::make_shared<SQLite::Database>(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    //_db.key("ABC55670788$"); //该版本sqlite3.c文件没有实现加密
    log_debug("db file {} opened!", db_path);
  } catch(std::exception& e) {
    log_error("open db file {} failed!,message:{}", db_path, str_system_to_utf8(e.what()));
    return false;
  }
  return true;
}

void DB::close()
{
  _db = nullptr;
  //nothing to do
  //db will be closed when _db destory
}

void DB::initTables()
{
  _createGlassConfigTable();
  _createBatchTable();
  _createFlawTable();
  _createStatisticsTable();
}

void DB::dropTables()
{
  _dropTable("glass_config_tb");
  _dropTable("batch_tb");
  _dropTable("flaw_tb");
  _dropTable("glass_statistics_tb");
}

void DB::dropTables2()
{
  //_dropTable("glass_config_tb");
  //_dropTable("batch_tb");
  _dropTable("flaw_tb");
  _dropTable("glass_statistics_tb");
}

bool DB::_dropTable(const std::string& tb)
{
  try {
    auto sql = std::string("DROP TABLE IF EXISTS ") + tb + ";";
    SQLite::Transaction t(*_db);
    _db->exec(sql);
    t.commit();
    log_info("drop table {} success!", tb);
    return true;
  } catch(std::exception& e) {
    log_error("drop table {} failed!,{}", tb, str_system_to_utf8(e.what()));
  }
  return false;
}

bool DB::_createTable(const std::string& tb, const std::string& sql)
{
  try {
    SQLite::Transaction t(*_db);
    _db->exec(sql);
    t.commit();
    log_info("create table {} success!", tb);
  } catch(std::exception& e) {
    log_error("create table {} failed!,{}", tb, str_system_to_utf8(e.what()));
  }
}

bool DB::_createGlassConfigTable()
{
  std::string sql = "CREATE TABLE IF NOT EXISTS [glass_config_tb] ("
                    "[config_name] NVARCHAR(255) NOT NULL PRIMARY KEY,"
                    "[json_txt] TEXT);";
  return _createTable("glass_config_tb", sql);
}

bool DB::_createBatchTable()
{
  std::string sql = "CREATE TABLE IF NOT EXISTS [batch_tb] ("
                    "[batch_id] NVARCHAR(255) NOT NULL  PRIMARY KEY,"
                    "[glass_config_name] NVARCHAR(255) NOT NULL,"
                    "[created_time] INTEGER NOT NULL);";
  return _createTable("batch_tb", sql);
}

bool DB::_createFlawTable()
{
  std::string sql = "CREATE TABLE IF NOT EXISTS [flaw_tb] ("
                    "[id] INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "[timestamp] INTEGER NOT NULL,"
                    "[batch_id] NVARCHAR(255) NOT NULL,"
                    "[glass_id] NVARCHAR(255) NOT NULL,"
                    "[cam_position] INTEGER NOT NULL,"
                    "[label] NVARCHAR(255) NOT NULL,"
                    "[score] REAL NOT NULL,"
                    "[area] REAL NOT NULL,"
                    "[realWidth] REAL NOT NULL,"
                    "[realHeight] REAL NOT NULL,"
                    "[detail_img_path] NVARCHAR(255),"
                    "[detail_nomark_img_path] NVARCHAR(255),"
                    "[audited] INTEGER,"
                    "[audited_result] INTEGER);"
                    "CREATE INDEX [flaw_tb_glass_id_index] ON [flaw_tb] ([glass_id]);"
                    "CREATE INDEX [flaw_tb_batch_id_index] ON [flaw_tb] ([batch_id]);"
                    "CREATE INDEX [flaw_tb_label_index] ON [flaw_tb] ([label]);";
  return _createTable("flaw_tb", sql);
}

bool DB::_createStatisticsTable()
{
  std::string sql1 = "CREATE TABLE IF NOT EXISTS [glass_statistics_tb] ("
                     "[glass_id] NVARCHAR(255) NOT NULL PRIMARY KEY,"
                     "[glass_config] NVARCHAR(255) NOT NULL,"
                     "[batch_id] NVARCHAR(255) NOT NULL,"
                     "[white_count] INTEGER NOT NULL,"
                     "[black_count] INTEGER NOT NULL,"
                     "[bright_count] INTEGER NOT NULL,"
                     "[dark_count] INTEGER NOT NULL,"
                     "[dirty_count] INTEGER NOT NULL,"
                     "[scratch_count] INTEGER NOT NULL);"
                     "CREATE INDEX [glass_statistics_tb_batch_id_index] ON [glass_statistics_tb] ([batch_id]);";


  std::string sql2 = "CREATE TABLE IF NOT EXISTS [all_statistics_tb] ("
                     "[id] INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "[detect_count] INTEGER NOT NULL,"
                     "[ok_count] INTEGER NOT NULL,"
                     "[ng_count] INTEGER NOT NULL);";

  std::string sql3 = "CREATE TABLE IF NOT EXISTS [batch_statistics_tb] ("
                     "[batch_id] NVARCHAR(255) NOT NULL PRIMARY KEY,"
                     "[detect_count] INTEGER NOT NULL,"
                     "[ok_count] INTEGER NOT NULL,"
                     "[ng_count] INTEGER NOT NULL);";

  return _createTable("glass_statistics_tb", sql1)
         && _createTable("all_statistics_tb", sql2)
         && _createTable("batch_statistics_tb", sql3);
}


std::vector<GlassConfig> DB::queryAllGlassConfigs()
{
  std::vector<GlassConfig> re;
  try {
    SQLite::Statement query(*_db, "select * from glass_config_tb;");
    while(query.executeStep()) {
      std::string name = query.getColumn(0).getString();
      std::string json = query.getColumn(1).getString();
      GlassConfig t;
      if(t.loadFromJsonStr(json)) {
        t.name = name;
        re.push_back(t);
      }
    }
  } catch(std::exception& e) {
    log_error("queryAllGlassConfigs failed! {}", str_system_to_utf8(e.what()));
  }
  return re;
}

std::vector<std::string> DB::queryAllGlassConfigNames()
{
  std::vector<std::string> re;
  try {
    SQLite::Statement query(*_db, "select * from glass_config_tb;");
    while(query.executeStep()) {
      std::string name = query.getColumn(0).getString();
      re.push_back(name);
    }
  } catch(std::exception& e) {
    log_error("queryAllGlassConfigNames failed! {}", str_system_to_utf8(e.what()));
  }
  return re;
}

bool DB::queryGlassConfigByName(const std::string& name, GlassConfig& re)
{
  try {
    QString sql = QString("select config_name,json_txt from glass_config_tb where config_name='%1';")
                  .arg(QString::fromStdString(name));
    SQLite::Statement query(*_db, sql.toStdString());
    if(query.executeStep()) {
      std::string name = query.getColumn(0).getString();
      std::string json = query.getColumn(1).getString();
      if(re.loadFromJsonStr(json)) {
        re.name = name;
        return true;
      }
    }
  } catch(std::exception& e) {
    log_error("queryGlassConfigByName :{} failed!,{}", name, str_system_to_utf8(e.what()));
  }
  return false;
}

bool DB::saveGlassConfig(const GlassConfig& config)
{
  std::string json_str;
  if(config.dumpToJsonStr(json_str)) {
    try {
      {
        log_debug("delete from glass_config_tb ...");
        SQLite::Statement cmd(*_db, "delete from glass_config_tb where config_name=:name;");
        cmd.bind(":name", config.name);
        cmd.exec();
      }

      {
        log_debug("inserting into glass_config_tb ...");
        SQLite::Statement cmd(*_db, "insert into glass_config_tb values(:name, :json);");
        cmd.bind(":name", config.name);
        cmd.bind(":json", json_str);
        if(cmd.exec() > 0) {
          log_debug("insert success!");
          return true;
        }
      }
    } catch(std::exception& e) {
      log_error("saveGlassConfig :{} failed! {}", config.name, str_system_to_utf8(e.what()));
    }
  }
  return false;
}

std::vector<Batch> DB::queryAllBatches()
{
  std::map<std::string, GlassConfig> configs_map;
  for(auto& t : queryAllGlassConfigs())
    configs_map[t.name] = t;

  std::vector<Batch> re;
  try {
    SQLite::Statement query(*_db, "select * from batch_tb order by created_time desc;");
    while(query.executeStep()) {
      std::string id = query.getColumn(0).getString();
      std::string config_name = query.getColumn(1).getString();
      int64_t timestamp = query.getColumn(2).getInt64();
      Batch b;
      b.id = id;
      b.timestamp = timestamp;
      b.glass_config = configs_map[config_name];
      re.push_back(b);
    }
  } catch(std::exception& e) {
    log_error("queryAllBatches failed! {}", str_system_to_utf8(e.what()));
  }
  return re;
}

bool DB::queryBatchById(const std::string& batch_id, Batch& re)
{
  try {
    QString sql = QString("select * from batch_tb where batch_id='%1';")
                  .arg(QString::fromStdString(batch_id));
    SQLite::Statement query(*_db, sql.toStdString());
    if(query.executeStep()) {
      std::string id = query.getColumn(0).getString();
      std::string config_name = query.getColumn(1).getString();
      int64_t timestamp = query.getColumn(2).getInt64();
      GlassConfig config;
      if(queryGlassConfigByName(config_name, config)) {
        re.id = id;
        re.glass_config = config;
        re.timestamp = timestamp;
        return true;
      }
    }
  } catch(std::exception& e) {
    log_error("queryBatchById :{} failed! {}", batch_id, str_system_to_utf8(e.what()));
  }
  return false;
}

bool DB::addBatch(const Batch& b)
{
  try {
    if(!isBatchExist(b.id)) {
      {
        log_debug("inserting into batch_tb ...");
        SQLite::Statement cmd(*_db, "insert into batch_tb values (:batch_id, :glass_config_name, :created_time);");
        cmd.bind(":batch_id", b.id);
        cmd.bind(":glass_config_name", b.glass_config.name);
        cmd.bind(":created_time", get_timestamp());
        if(cmd.exec() > 0) {
          log_debug("insert success!");
          return true;
        }
      }
    }
  } catch(std::exception& e) {
    log_error("addBatch :{} failed! {}", b.id, str_system_to_utf8(e.what()));
  }
  return false;
}

bool DB::addFlaw(const FlawRecord& re)
{
  try {
    QString sql = QString("insert into flaw_tb values (NULL,:timestamp,:batch_id,:glass_id,:cam_position,:label,%1,%2,%3,%4,:detail_img_path,:detail_nomark_img_path,:audited,:audited_result);").arg(re.score).arg(re.area).arg(re.realWidth).arg(re.realHeight);

    log_debug("inserting into flaw_tb ...");
    SQLite::Statement cmd(*_db, sql.toStdString());
    cmd.bind(":timestamp", re.timestamp);
    cmd.bind(":batch_id", re.batch_id);
    cmd.bind(":glass_id", re.glass_id);
    cmd.bind(":cam_position", (int)re.cam_position);
    cmd.bind(":label", re.label);
    cmd.bind(":detail_img_path", re.detail_img_path);
    cmd.bind(":detail_nomark_img_path", re.detail_nomark_img_path);
    cmd.bind(":audited", re.audited);
    cmd.bind(":audited_result", re.audited_result);

    if(cmd.exec() > 0) {
      log_debug("insert success!");
      return true;
    }
  } catch(std::exception& e) {
    log_error("addFlaw failed! {}", str_system_to_utf8(e.what()));
  }
  return false;
}

bool DB::addGlassToStatistics(const std::string& batch_id, std::shared_ptr<GlassObject> glass)
{
  if(!glass)
    return false;

  try {
    GlassStatisticsRecord gre;
    gre.glass_id = glass->id();
    gre.glass_config = glass->config().name;
    gre.batch_id = batch_id;
    gre.white_count = 0;
    gre.black_count = 0;
    gre.bright_count = 0;
    gre.dark_count = 0;
    gre.dirty_count = 0;
    gre.scratch_count = 0;
    for(auto& f : glass->allFlaws()) {
      if(f.label == "white") gre.white_count++;
      if(f.label == "black") gre.black_count++;
      if(f.label == "bright") gre.bright_count++;
      if(f.label == "dark") gre.dark_count++;
      if(f.label == "dirty") gre.dirty_count++;
      if(f.label == "scratch") gre.scratch_count++;
    }

    bool all_statistics_record_exist = false;
    {
      SQLite::Statement query(*_db, "select * from all_statistics_tb;");
      if(query.executeStep())
        all_statistics_record_exist = true;
    }
    if(!all_statistics_record_exist) {
      SQLite::Statement cmd(*_db, "insert into all_statistics_tb values (NULL,0,0,0);");
      cmd.exec();
    }
    AllStatisticsRecord are;
    _queryAllStatisticsRecord(are);

    //判断批次记录是否存在，否则创建批次记录
    bool batch_statistics_record_exist = false;
    {
      SQLite::Statement query(*_db, "select * from batch_statistics_tb where batch_id=:batch_id;");
      query.bind(":batch_id", batch_id);
      if(query.executeStep())
        batch_statistics_record_exist = true;
    }
    if(!batch_statistics_record_exist) {
      SQLite::Statement cmd(*_db, "insert into batch_statistics_tb values (:batch_id,0,0,0);");
      cmd.bind(":batch_id", batch_id);
      cmd.exec();
    }
    BatchStatisticsRecord bre;
    _queryBatchStatisticsRecord(batch_id, bre);

    bool ok = glass->isOk();
    if(!ok) { //有缺陷的玻璃记录
      SQLite::Statement cmd(*_db, "insert into glass_statistics_tb values (:glass_id,:glass_config,:batch_id,:white_count,:black_count,:bright_count,:dark_count,:dirty_count,:scratch_count);");
      cmd.bind(":glass_id",        gre.glass_id);
      cmd.bind(":glass_config",    gre.glass_config);
      cmd.bind(":batch_id",        gre.batch_id);
      cmd.bind(":white_count",     gre.white_count);
      cmd.bind(":black_count",     gre.black_count);
      cmd.bind(":bright_count",    gre.bright_count);
      cmd.bind(":dark_count",      gre.dark_count);
      cmd.bind(":dirty_count",     gre.dirty_count);
      cmd.bind(":scratch_count",   gre.scratch_count);
      cmd.exec();
    }

    //更新批次统计
    {
      SQLite::Statement cmd(*_db, "update batch_statistics_tb set detect_count=:detect_count,ok_count=:ok_count,ng_count=:ng_count where batch_id=:batch_id;");
      cmd.bind(":batch_id",        batch_id);
      cmd.bind(":detect_count",    bre.detect_count + 1);
      cmd.bind(":ok_count",        ok ? bre.ok_count + 1 : bre.ok_count);
      cmd.bind(":ng_count",        ok ? bre.ng_count : bre.ng_count + 1);
      //更新缓存
      if(_batch_cache.count(batch_id) > 0) {
        auto& bb = _batch_cache[batch_id];
        bb.detect_count = bre.detect_count + 1;
        bb.ok_count = ok ? bre.ok_count + 1 : bre.ok_count;
        bb.ng_count = ok ? bre.ng_count : bre.ng_count + 1;
      }
      //更新数据库
      cmd.exec();
    }

    //更新总检统计
    {
      SQLite::Statement cmd(*_db, "update all_statistics_tb set detect_count=:detect_count,ok_count=:ok_count,ng_count=:ng_count;");
      cmd.bind(":detect_count",    are.detect_count + 1);
      cmd.bind(":ok_count",        ok ? are.ok_count + 1 : are.ok_count);
      cmd.bind(":ng_count",        ok ? are.ng_count : are.ng_count + 1);
      //更新缓存
      if(_all_cached) {
        _all_cache.detect_count = are.detect_count + 1;
        _all_cache.ok_count = ok ? are.ok_count + 1 : are.ok_count;
        _all_cache.ng_count = ok ? are.ng_count : are.ng_count + 1;
      }
      //更新到数据库
      cmd.exec();
    }
    return true;
  } catch(std::exception& e) {
    log_error("update all_statistics_tb failed! {}", str_system_to_utf8(e.what()));
  }
  return false;
}

bool DB::_queryBatchStatisticsRecord(const std::string& batch_id, BatchStatisticsRecord& re)
{
  try {
    SQLite::Statement query(*_db, "select * from batch_statistics_tb where batch_id=:batch_id;");
    query.bind(":batch_id", batch_id);
    if(query.executeStep()) {
      re.batch_id = batch_id;
      re.detect_count = query.getColumn(1).getInt();
      re.ok_count = query.getColumn(2).getInt();
      re.ng_count = query.getColumn(3).getInt();
      _batch_cache[batch_id] = re; //缓存
      return true;
    }
  } catch(std::exception& e) {
    log_error("_queryBatchStatisticsRecord :{} failed!,{}", batch_id, str_system_to_utf8(e.what()));
  }
  return false;
}

bool DB::_queryAllStatisticsRecord(AllStatisticsRecord& re)
{
  try {
    SQLite::Statement query(*_db, "select * from all_statistics_tb;");
    if(query.executeStep()) {
      re.detect_count = query.getColumn(1).getInt();
      re.ok_count = query.getColumn(2).getInt();
      re.ng_count = query.getColumn(3).getInt();

      if(!_all_cached) {
        _all_cache = re;
        _all_cached = true;
      }
      return true;
    }
  } catch(std::exception& e) {
    log_error("_queryAllStatisticsRecord failed!,{}", str_system_to_utf8(e.what()));
  }
  return false;
}

bool DB::queryAllStatisticsRecord(AllStatisticsRecord& re)
{
  if(_all_cached) {
    re = _all_cache;
    return true;
  }

  return _queryAllStatisticsRecord(re);
}

bool DB::queryBatchStatisticsRecord(const std::string& batch_id, BatchStatisticsRecord& re)
{
  if(_batch_cache.count(batch_id) > 0) {
    re = _batch_cache[batch_id];
    return true;
  }

  return _queryBatchStatisticsRecord(batch_id, re);
}
