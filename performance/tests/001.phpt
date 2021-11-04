--TEST--
Check if performance is loaded
--SKIPIF--
<?php
if (!extension_loaded('performance')) {
    echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "performance" is available';
?>
--EXPECT--
The extension "performance" is available
