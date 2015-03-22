# Hide Me Fatty
<pre>
<a href="#about">About</a>
<a href="#latest-version">Latest version</a>
<a href="#download">Downloads</a>
<a href="#before-hide-and-seek">Before hide-and-seek</a>
<a href="#usage">Usage</a>
	<a href="#usage-general">General info</a>
	<a href="#usage-hidding">Hidding example</a>
	<a href="#usage-restoring">Restoring example</a>
</pre>

## About
Hide Me Fatty is simple program that is able to hide files on the partition with (for now only) FAT32 filesystem.

## Latest version
Latest version is v0.3. <br />
Like in linux kernel odd minor (v0.>>3<<) represent beta release. Even represent stable version.<br />
So now you know that current release is beta. If you want help me with finding bugs you are free to <a href="#download-debug-version">download debug version</a>. That version generate lots of logs (located in files/logs/global.log) which may help me with bugs.<br />

## Downloads

## Before hide-and-seek
There are some things you must to know before using this program.<br />
First of all you must run it with root permissions.<br />
After hidding some files you can view/copy etc. files on partition (more about that below) but you can't modify/remove them because you will lose hidded files.<br />

## Usage
### General info
Program will ask you for four things.<br />
1. Files to hide - paths to files you want to hide.<br />
2. Files on partition - FULL paths to files on partition after what you want to hide your files.<br />
3. Path to partition - FULL path to partition. On most linux it is that in /media folder.<br />
4. Path to partition device file - FULL path to partition device file. That on /dev folder.<br />

### Hidding example
<code>

</code>

### Restoring example
<code>

</code>