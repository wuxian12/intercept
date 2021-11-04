--TEST--
test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('performance')) {
    echo 'skip';
}
?>
--FILE--
<?php
$ret = test1();

var_dump($ret);
?>
--EXPECT--
The extension performance is loaded and working!
NULL
