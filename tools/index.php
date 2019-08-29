<?php
$action = $_GET['action'];
set_time_limit(3);
?>

<!doctype html>
<html>
<head>
<meta charset="UTF-8">
<title>rpi-beamertool webinterface</title>
<style type="text/css">
body {
    text-align:center;
    font-family:Arial;
}
button {
    font-size:20px;
    width:250px;
    height:100px;
}
</style>
</head>
<body>

<h1>Raspberry Pi Beamertool - Webinterface</h1>

<button onClick="self.location.href='index.php?action=restart'">Restart RPI Beamertool</button><br />
<button onClick="self.location.href='index.php?action=reboot'">Reboot Raspberry Pi</button><br />
<button onClick="self.location.href='index.php?action=shutdown'">Shutdown Rasbperry Pi</button><br />

<?php if ($action == "restart") { ?>
<script type="text/javascript">
    <!--
    window.location="index.php"
    //-->
</script>
<?php } ?>

</body>
</html>
<?php
if ($action == "reboot") {
    shell_exec("sudo /sbin/shutdown -r now");
} elseif ($action == "shutdown") {
    shell_exec("sudo /sbin/shutdown -h now");
} elseif ($action == "restart") {
    system("sudo /opt/rpi-beamertool/rpi-bt-restart.sh > /dev/null &");
}
?>
