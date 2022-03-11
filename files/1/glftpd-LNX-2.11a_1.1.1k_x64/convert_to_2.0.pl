#!/usr/bin/perl
# NOTE: if your perl is elsewhere, either change the first line to where the
# binary is or execute like this: /path/to/perl ./convert_to_2.0.pl

### convert_to_2.0.pl by Usurper, October-December 2002
#
# Syntax: ./convert_to_2.0.pl [/glftpd_path]
#
# If no path is specified, /glftpd is assumed
#
# Run this as root, or create 'groups' and 'users-new' in ftp-data and chmod them to 777
#
#####################################################

if (length $ARGV[0]) {
  $glftpd_dir = "$ARGV[0]";
} else {
  $glftpd_dir = "/glftpd";
}
if (not -d "$glftpd_dir") {
  die "Syntax: $0 /path_to_glftpd\n";
}

print "Glftpd is in $glftpd_dir\n";

$etcgroup = "$glftpd_dir/etc/group";

if (not -s "$etcgroup") {
  die("Error: the etc/group file could not be found!\n");
}

open GROUP, "$etcgroup" or die "Can't read $etcgroup: $!\n";
@grouplines = <GROUP>;
close GROUP;

$userdir = "$glftpd_dir/ftp-data/users";
$newuserdir = "$glftpd_dir/ftp-data/users-new";
if (not -d "$newuserdir") {
  print "Creating $newuserdir...\n";
  mkdir("$newuserdir") or die "Error: $!\n";
} 

@userfiles = `ls $userdir |grep -v default`;
foreach $user (@userfiles) {
  chomp $user;
  next if ($user =~ m/\.lock$/);
  undef $primarygroup;
  undef $flags;
  undef $slot1, $slot2;
  open USER, "$userdir/$user" or die "Can't read userfile $user: $!\n";
  open USEROUT, ">$newuserdir/$user" or die "Can't create userfile $newuserdir/$user: $!\n";
  while ($line = <USER>) {
    if ($line =~ /^FLAGS (.*)$/) {
      $flags = $1;
      print USEROUT $line;
    } elsif ($line =~ /^GROUP (.*?) (.*?)$/) {
      print "Warning: userfile for $user already in NEW format!\n";
      if (!defined($primarygroup)) {
        $primarygroup = $1;
      }
      print USEROUT $line;
    } elsif ($line =~ /^GROUP (.*)$/) {
      if (!defined($primarygroup)) {  #this is the first group
        $primarygroup = $1;
        if ($flags =~ m/2/) {  #this is a gadmin
          chomp $line;
          print USEROUT "$line 1\n";
        } else {  # not a gadmin
          print USEROUT $line;
        }
      } else {  # not the first group
        print USEROUT $line;
      }
    } elsif ($line =~ /^SLOTS (.*?) (.*?)$/) {
      $slot1 = $1; $slot2 = $2;
      # eat this line
    } else {
      print USEROUT $line;
    }
  }  # while
  close USER;
  close USEROUT;
  print "New userfile $newuserdir/$user successfully created.\n";
  if ($flags =~ m/2/ and length($primarygroup) and ($slot1 > 0)) {
    if ($slot1 > 0) {
      print "Adding $user" . "'s $slot1 group slots";
      if ($slot2 > 0) {
        print " and $slot2 leech slots";
      } else {
        $slot2 = 0;
      }
      print " to group $primarygroup\n";
    } else {
      $slot1 = 0;
    }
    if (defined($groupslots{$primarygroup})) {
      ($s1, $s2) = split /:/, $groupslots{$primarygroup};
    } else {
      $s1 = $s2 = 0;
    }
    $s1 += $slot1;
    $s2 += $slot2;
    $groupslots{$primarygroup} = "$s1:$s2";
  }
}

print "Copying default files to $newuserdir...";
if (`cp $userdir/default.* $newuserdir/.`) {
  print "ERROR: $!";
}
print "\n";

$groupdir = "$glftpd_dir/ftp-data/groups";
if (not -d "$groupdir") {
  print "Creating $groupdir...\n";
  mkdir("$groupdir") or die "Error: $!\n";
} 

print "Now creating groupfiles...\n";
foreach $groupline (@grouplines) {
  chomp $groupline;
  ($group,$groupnfo,$gid,$groupslots) = split /:/, $groupline;
  if (-s "$groupdir/$group") {
    print "$group already exists in $groupdir, skipping...\n";
    next;
  }
  open GROUP, ">$groupdir/$group" or die "Can't create $groupdir/$group: $!\n";
  if (not length($groupslots)) {
    $groupslots = 0;
  }
  print GROUP "SIMULT $groupslots\n";
  print GROUP "GROUPNFO $groupnfo\n";
  $vals = $groupslots{$group};
  if (defined($vals)) {
    $vals =~ s/:/ /;
    print GROUP "SLOTS $vals -1 -1\n";
  }
  close GROUP;
  print "$groupdir/$group successfully created.\n";
}

print "All done\n\nNow go through the new userfiles (in $newuserdir) and groupfiles (in $groupdir) " .
 "and make sure everything is ok. If it is, rename $userdir to $userdir.bak and $newuserdir " .
 "to $userdir, which will put these changes into effect. Be sure the old glftpd binary is " .
 "disabled when you do this, as it will undo these changes if executed!\n";

