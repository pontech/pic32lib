#!/usr/bin/perl
use strict;

print "MPIDE Library Relocator v0.1\n";
print "brian\@pontech.com\n\n";

use File::HomeDir;
use File::Basename;
use File::Copy;

my $sourceDir = dirname($0);
my $baseName = basename($0);
my $homeDir = File::HomeDir->my_home;
my $destinationDir = "$homeDir\\Workspace\\Mpide\\libraries";
my $stateFile = "DO_NOT_COMMIT.txt";

#print dir($sourceDir);

print "S: ", $sourceDir, "\n";
print "D: ", $destinationDir, "\n";
print "\n\n";

unless($baseName eq "Relocate.pl") {
    print "Basename Mismatch\n";
}
else {
    if(-e $stateFile) {
        print "D -> S\n";
        open(FILE, "$stateFile");
        while(<FILE>) {
            chomp;
            my $fileName = basename($_);
#            print "mv $_ $sourceDir\\$fileName\n";
            move($_, "$sourceDir\\$fileName");
            rmdir(dirname($_));
        }
        close(FILE);
        unlink $stateFile;
    }
    else {
        print "S -> D\n";
        open(FILE, ">>$stateFile");

        opendir(my $dh, $sourceDir) || die "can't opendir $sourceDir: $!";
        my @files = grep(/\.h$/, readdir($dh));
        closedir($dh);

        foreach my $fileName(@files) {
            (my $sansExtension = $fileName) =~ s/\.[^.]+$//;
            mkdir "$destinationDir\\$sansExtension", 0755;
#            print "mv $sourceDir\\$fileName $destinationDir\\$sansExtension\\$fileName\n";
            move("$sourceDir\\$fileName", "$destinationDir\\$sansExtension\\$fileName");
            print FILE "$destinationDir\\$sansExtension\\$fileName\n";
        }

        close(FILE);
    }
}

#print "\n<Press Enter to Close>\n";
#<STDIN>;
