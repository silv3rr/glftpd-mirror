# This is just a sample tcl for you to have as an example.
# It should work if you load it on your bot after compiling the
# binary.

# What channel do we invite to?
set chan "#my_site_chan";

bind msg - !inv pub_invite

proc pub_invite { nick uhost hand args } { global botnick chan
   set args [string trim [string trim $args \{ ] \}]
   set arglist [llength [split $args]]
   if { $arglist != 2 } {
      putserv "PRIVMSG $nick :/msg $botnick invite <user> <password>"
      return;
   }
   set user [lindex [split $args] 0]
   set pass [lindex [split $args] 1]
   if {[catch {set login [open "|/glftpd/bin/passchk $user $pass"]}] != 0} {
      putlog "ERROR - Couldn't execute passchk."
      putlog "Is speed located in the bin dir and is it chmoded?"
      close $login; return;
   }
   set line [gets $login]; close $login;
   if { $line == "1" } {
      putserv "INVITE $nick $chan"
      putserv "PRIVMSG $chan :Invited $nick."
   } else {
      putserv "PRIVMSG $chan :$nick failed to get invite."
   }
}
