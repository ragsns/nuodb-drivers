<?php 
try {  
  $db = new PDO("nuodb:database=test@localhost;schema=Hockey", "cloud", "user") or die;
  $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  $player_number = 12;
  $sql = "select * from hockey where NUMBER = :number";
  $stmt = $db->prepare($sql);
  $stmt->bindParam(':number', $player_number, PDO::PARAM_INT);
  $stmt->execute();
  $result = $stmt->fetchAll();
  foreach ($result as $row) {
     print_r ($row);
  }
  $db = NULL;
} catch(PDOException $e) {  
  echo $e->getMessage();  
}
$db = NULL;  
echo "done\n";
?>