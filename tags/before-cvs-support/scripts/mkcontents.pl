#!/usr/bin/perl -w
# mkcontents

#use strict qw(refs subs);

sub openlevel;
sub closelevel;
sub newindex;
sub doneindex;
sub html;
sub tag;
sub out;

$output_level = 0;
$output = 0;

# read input files
open(INDEX, "<INDEX") || die "open INDEX: $!";
while (defined($filename = <INDEX>)) {
    chomp $filename;
    open(FILE, "<$filename") || die "open $filename: $!";
    {
        local $/ = undef;
        $filetext = <FILE>;
        print STDERR "$filename:\n";
        $output = 0;
        html($filetext);
    }
    close(FILE);
}
close(INDEX);
closelevel(0);

sub html {
    my $text = $_[0];
    my $cpos = 0;

    TAG: while ($opos = $cpos, ($cpos = index($text, "<", $cpos) + 1) != 0) {
        pos($text) = $cpos;  # match regexp there

        out(substr($text, $opos, pos($text) - $opos - 1));
        # output text

        $TAG = undef;
        
        if ($text =~ /\G!(.*?)(--.*?--\s*)*(.*?)>/sgo) { # comment
            $cpos = pos $text;
            #&comment($2);
            next TAG;
        }
        
        pos ($text) = $cpos;
        if ($text =~ /\G\s*([\/]?[A-Za-z0-9]*)/go) {
            $cpos = pos($text);
            $TAG = uc $1;
            
            #print "<|", $TAG, "\n";
        }
        
        undef %ARGS;
        
        ARG: while (1) {
            pos($text) = $cpos;
            
            if ($text =~ /\G\s*/go) { $cpos = pos ($text); } # skip whitespace
            
            last ARG unless $text =~ /\G([A-Za-z0-9]+)\s*/go; # param name
            $cpos = pos $text;
            
            $pname = uc $1;
            if ($text =~ /\G=\s*/go) {
                $cpos = pos $text;
                
                if ($text =~ /\G"([^"]*)"\s*/go) {
                    $cpos = pos $text;
                    #print "+|$pname=\"$1\"\n";
                    $ARGS{$pname} = $1;
                    next ARG;
                };
                pos($text) = $cpos;
                if ($text =~ /\G'([^']*)'\s*/go) {
                    $cpos = pos $text;
                    #print "+|$pname='$1'\n";
                    $ARGS{$pname} = $1;
                    next ARG;
                };
                pos($text) = $cpos;
                if ($text =~ /\G([^ <>"']+)\s*/go) {
                    $cpos = pos $text;
                    #print "+|$pname=$1\n";
                    $ARGS{$pname} = $1;
                    next ARG;
                };
                $ARGS{$pname} = "";
                die "no value for tag";
            }
            #print "+|$pname\n";
        }
        pos($text) = $cpos;
        ($cpos = index($text, ">", $cpos) + 1) != 0 or die "tag without end";
        
        tag($TAG, \%ARGS);
    }
    out(substr($text, $opos, length($text) - $opos));
}

sub closelevel {
    my $level = $_[0];
    
    while ($output_level > $level) {
        print "\n</DL>\n";
        $output_level--;
    }
}

sub openlevel {
    my $level = $_[0];

    while ($output_level < $level) {
        print "<DL>\n";
        $output_level++;
    }
}

sub newindex {
    my $level = $_[0];

    closelevel($level);
    openlevel($level);
    print "\n<LI>";
    $output = 1;
}

sub doneindex {
    my $level = $_[0];

    warn "$filename:level:$level mismatch" if ($output_level != $level);
    print "</LI>\n";
    $output = 0;
}

sub tag {
    my $TAG = $_[0];
    my %ARGS = %{$_[1]};

    $TAG eq 'TITLE' && do {
        newindex(1);
        print "<A HREF=\"$filename\" TARGET=\"main\">\n";
    }
    or $TAG eq "/TITLE" && do {
        print "</A>";
        doneindex(1);
    }
    or $TAG =~ /^H([1-6])$/o && do {
        newindex(1 + $1);
    }
    or $TAG =~ /^\/H([1-6])$/o && do {
        doneindex(1 + $1);
    }
    or $TAG eq 'A' && do {
        if (defined $ARGS{"NAME"}) {
            $aname = $filename . '#' . $ARGS{"NAME"};
            print "<A HREF=\"$aname\" TARGET=\"main\">" if $output;
        } 
    }
    or $TAG eq '/A' && do {
        print "</A>" if $output;
    }
}

sub out {
    my $lin = $_[0];
    my $first = 1;
    my $i;

    print $lin if $output;
}

sub comment {
}

sub badtag {
}
