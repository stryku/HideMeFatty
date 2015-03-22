# Hide Me Fatty
<pre>
<a href="#about">About</a>
<a href="#latest-version">Latest version</a>
<a href="#in-case-you-have-too-much-free-time">In case you have too much free time</a>
<a href="#downloads">Downloads</a>
<a href="#before-hide-and-seek">Before hide-and-seek</a>
<a href="#usage">Usage</a>
	<a href="#general-info">General info</a>
	<a href="#hiding-example">Hiding example</a>
	<a href="#restoring-example">Restoring example</a>
</pre>

## About
Hide Me Fatty is simple program that is able to hide files on the partition with (for now only) FAT32 filesystem.

## Latest version
Latest version is v0.3. <br />
Like in linux kernel odd minor (v0.>>3<<) represent beta release. Even represent stable version.<br />

## In case you have too much free time
So now you know that current release is beta. If you want help me with finding bugs you are free to <a href="#downloads">download</a> and play with version with log generation (that in "release-logs" folder). That version generate lots of logs (located in files/logs/global.log) which may help me with bugs. Then when program crashes you can send me this log via email (stryku2393( at )gmail.com). I will be grateful for your help.<br />

## Downloads
Last release: https://sourceforge.net/projects/hidemefatty/

## Before hide-and-seek
There are things you must to know before using this program.<br />
First of all you need run it with root permissions.<br />
After hiding some files you can view/copy etc. files on partition (more about that below) but you can't modify/remove them because you will lose hidded files.<br />

## Usage
### General info
Program will ask you for four things.<br />
1. Files to hide - paths to files you want to hide.<br />
2. Files on partition - FULL paths to files on partition after what you want to hide your files.<br />
3. Path to partition - FULL path to partition. On most linux it is that in /media folder.<br />
4. Path to partition device file - FULL path to partition device file. That on /dev folder.<br />

### Hiding example
<pre>
<code>
root@centrum-dowodzenia:/tmp/test/HideMeFatty-v0.3/release# ./HideMeFatty-v0.3-release 


		MENU

1. Hide files
2. Restore files
0. Exit
>1

Put paths to files to hide. And 'q' after that

>test.png
>anotherTest.png
>test/test.png
>q

Put full paths to files on partiiton. And 'q' after that.
See documentation for details

>/media/stryku/GIS/test-hideAfterMe.xz
>/media/stryku/GIS/test-hideAfterMe.bmp
>/media/stryku/GIS/test-hideAfterMe.txt
>q

Put full path to partition (that in /media folder)

>/media/stryku/GIS

Put full path to partition device (that in /dev/ folder)

>/dev/sdb1

Starting hiding files. Be patient...
Files hidded
Don't forget to unmount parition


		MENU

1. Hide files
2. Restore files
0. Exit
>0
Bye bye
</code>
</pre>

### Restoring example
<pre>
<code>
root@centrum-dowodzenia:/tmp/test/HideMeFatty-v0.3/release# ./HideMeFatty-v0.3-release 


		MENU

1. Hide files
2. Restore files
0. Exit
>2

Put full paths to files on partiiton. And 'q' after that.
See documentation for details

>/media/stryku/GIS/test-hideAfterMe.bmp
>/media/stryku/GIS/test-hideAfterMe.xz
>/media/stryku/GIS/test-hideAfterMe.txt
>q

Put full path to partition (that in /media folder)

>/media/stryku/GIS

Put full path to partition device (that in /dev/ folder)

>/dev/sdb1

Put path where store restored files

>restored

Starting restoring files. Be patient...
Files restored.

		MENU

1. Hide files
2. Restore files
0. Exit
>0
Bye bye
</code>
</pre>

 And as you can se below we have our hidded files in "restored" folder
<pre>
<code>
root@centrum-dowodzenia:/tmp/test/HideMeFatty-v0.3/release# ls restored/
anotherTest.png  test.png  test (1).png
</code>
</pre>
