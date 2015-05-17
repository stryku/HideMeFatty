# Hide Me Fatty
<pre>
<a href="#about">About</a>
<a href="#latest-version">Latest version</a>
<a href="#downloads">Downloads</a>
<a href="#before-hide-and-seek">Before hide-and-seek</a>
	<a href="#chown-device-file">Chown device file</a>
	<a href="#but-there-is-a-warning">There is a warning!</a>
<a href="#usage">Usage</a>
	<a href="#running-program">Running program</a>
	<a href="#hiding">Hiding example</a>
	<a href="#restoring">Restoring example</a>
</pre>

## About
Hide Me Fatty is simple program that is able to hide files on the partition with (for now only) FAT32 file system.

## Latest version
Latest version is v1.0 <br /> That version is still in tests. If you have any problems with program I will be grateful for some details.

## Downloads
Latest release: https://github.com/stryku/HideMeFatty/releases/tag/v1.0

## Before hide-and-seek
There are things you must know before using this program.<br />
###Chown device file
Before hiding or restoring files you must prepare partition device file. In this example, linux user name is "stryku" and device label i "HideMeFatty"

Follow this steps:
<pre>
1. Open terminal and type "cat /etc/mtab"
2. Find your partition line
3. Type "sudo chown yourLinuxUsername:yourLinuxUsername firstThingInLine" for example "sudo chown stryku:stryku /dev/sdb1"
</pre>

All that should look like this:
<pre>
<code>
<b>stryku@cedtrumDowodzenia:~$</b> cat /etc/mtab
/dev/sda6 / ext4 rw,errors=remount-ro 0 0
proc /proc proc rw,noexec,nosuid,nodev 0 0
sysfs /sys sysfs rw,noexec,nosuid,nodev 0 0
none /sys/fs/cgroup tmpfs rw 0 0
none /sys/fs/fuse/connections fusectl rw 0 0
none /sys/kernel/debug debugfs rw 0 0
none /sys/kernel/security securityfs rw 0 0
udev /dev devtmpfs rw,mode=0755 0 0
devpts /dev/pts devpts rw,noexec,nosuid,gid=5,mode=0620 0 0
tmpfs /run tmpfs rw,noexec,nosuid,size=10%,mode=0755 0 0
none /run/lock tmpfs rw,noexec,nosuid,nodev,size=5242880 0 0
none /run/shm tmpfs rw,nosuid,nodev 0 0
none /run/user tmpfs rw,noexec,nosuid,nodev,size=104857600,mode=0755 0 0
none /sys/fs/pstore pstore rw 0 0
binfmt_misc /proc/sys/fs/binfmt_misc binfmt_misc rw,noexec,nosuid,nodev 0 0
systemd /sys/fs/cgroup/systemd cgroup rw,noexec,nosuid,nodev,none,name=systemd 0 0
gvfsd-fuse /run/user/1000/gvfs fuse.gvfsd-fuse rw,nosuid,nodev,user=stryku 0 0
/dev/sda2 /media/stryku/CA807D94807D8829 fuseblk rw,nosuid,nodev,allow_other,default_permissions,blksize=4096 0 0
/dev/sdb1 /media/stryku/HideMeFatty vfat rw,nosuid,nodev,uid=1000,gid=1000,shortname=mixed,dmask=0077,utf8=1,showexec,flush,uhelper=udisks2 0 0
<b>stryku@cedtrumDowodzenia:~$</b> sudo chown stryku:stryku /dev/sdb1
[sudo] password for stryku: 
<b>stryku@cedtrumDowodzenia:~$</b>
</code>
</pre>

Now you can hide and restore files

#But there is a warning!
After hiding some files you can view/copy etc. files on partition after which you hid your files but you can't modify/remove them because you will lose hided files.<br />

## Usage

### Running program
To run prorgam just type 
<pre>
<code>
cd /path/to/downloaded/HideMeFatty
./start.run
</code>
</pre>


### Hiding
####Step 1
In this step you must select partition from list. In combobox will be partitions with valid FAT32 file system.

####Step 2
Next you must select files on partition after which you want to hide your files. Below the table you can see amount of free space where you can hide files. Total size of files + metadata can not be higher than that value.

####Step 3
Here just select files you want to hide. Again, below the table you can see total size to hide (size of files + size of metada).

####Step 4
Simple. Click button and be patient.

### Restoring
####Step 1
Same as step 1 in hiding section

####Step 2
Here you must select files on partition after which you once hid your files.

####Step 3
Select where you want to store restored files

####Step 4
Not much to do. Just click button.

