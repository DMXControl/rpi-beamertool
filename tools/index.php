<?php

$action = $_GET['action'];
set_time_limit(1);


?>


<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
       "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>LaserPi</title>
</head>
<body>

<h1>LaserPi Simple WebInterface</h1>

<button onClick="self.location.href='index.php?action=restart'">Restart LaserSim</button>
<button onClick="self.location.href='index.php?action=reboot'">Reboot RaspberryPi</button>
<button onClick="self.location.href='index.php?action=shutdown'">Power Off RasbperryPi</button>


<?php
if ($action == "restart")
{
    echo "<script type=\"text/javascript\">";
    echo "<!--";
    echo "window.location=\"index.php\"";
    echo "//-->";
    echo "</script>";
}
?>


</body>
</html>

<?php
if ($action == "reboot")
{
    shell_exec("sudo /sbin/shutdown -r now");
}

if ($action == "shutdown")
{
    shell_exec("sudo /sbin/shutdown -h now");
}

if ($action == "restart")
{
    system("sudo /usr/local/bin/lasersim > /dev/null &");
}

?>