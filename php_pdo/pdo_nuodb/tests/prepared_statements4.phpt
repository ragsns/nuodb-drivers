--TEST--
Test insert prepared statements.
--FILE--
<?php 
// Test insert prepared statements.

$db = NULL;  

function open_db() {
  $db = new PDO("nuodb:database=test@localhost;schema=Hockey", "dba", "goalie") or die;
  return $db;
}

function create_table_hockey_test() {
  print("create table Hockey_test\n");
  try {  
    $db = open_db();
    $sql = 'create table Hockey_test ( Id Integer not NULL generated always as identity primary key, Number Integer, Name String, Position String, Team String);';
    $count = $db->exec($sql);
    $sql = 'create unique index player_idx_test on Hockey_test (Number, Name, Team);';
    $count = $db->exec($sql);
    $sql = 'insert into Hockey_test select * from Hockey;';
    $count = $db->exec($sql);
    $db = NULL;
  } catch(PDOException $e) {
    print ("Failed\n");  
    echo $e->getMessage();  
  }
  $db = NULL;  
}

function drop_table_hockey_test() {
  print("drop table Hockey_test\n");
  try {  
    $db = open_db();
    $sql = "drop table Hockey_test;";
    $count = $db->exec($sql);
    $db = NULL;
  } catch(PDOException $e) {  
    echo $e->getMessage();  
  }
  $db = NULL;  
}

try {  
  $db = new PDO("nuodb:database=test@localhost;schema=Hockey", "dba", "goalie") or die;
  $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  $position = NULL;
  create_table_hockey_test();
  $sql = "insert into Hockey_test(NUMBER, NAME, POSITION, TEAM) values (101, 'Tom Gates', NULL, NULL)";
  $stmt = $db->prepare($sql);
  $stmt->execute();
  $db->commit();
  $sql = "select count(*) from Hockey_test where POSITION = :position";
  $stmt = $db->prepare($sql);
  $stmt->bindParam(':position', $position, PDO::PARAM_STR);
  $stmt->execute();
  $result = $stmt->fetchAll();
  foreach ($result as $row) {
     print_r ($row);
  }
  $db = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
drop_table_hockey_test();
$db = NULL;  
echo "done\n";
?>
--EXPECT--
create table Hockey_test
Array
(
    [COUNT] => 0
    [0] => 0
)
drop table Hockey_test
done
