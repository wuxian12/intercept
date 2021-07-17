--TEST--
Check if pid is loaded
--SKIPIF--
<?php
if (!extension_loaded('pid')) {
    echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "pid" is available';
?>
--EXPECT--
The extension "pid" is available
