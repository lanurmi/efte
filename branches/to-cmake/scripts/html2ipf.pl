#!perl5 -w

# html2ipf - version 0.5
# by Marko Macek, mark@hermes.si | marko.macek@snet.fer.uni-lj.si
# needs some work, but is much faster than 0.2.

# version 0.4: handle some internal A HREF and A NAME.
# version 0.5 changed things to make it work with multiple html files.

use strict qw(refs subs);

$h2i = 'html2ipf';
$h2i_version = '0.4';

print ".*! $h2i $h2i_version\n\n";

$dl_param = 'compact tsize=10 break=all';
$ol_param = 'compact';
$ul_param = 'compact';
$lmargin  = ':lm margin=1.';
$fontspec = ''; #:font facename=Helv size=16x8.';

undef $/; # slurp whole file as one line

# read input files
$file_count = 0; 
while (<>) {
    $file_name[$file_count] = $ARGV;
    $file_text[$file_count] = $_;
    printf STDERR
        "file:%s size:%s\n",
        $file_name[$file_count],
        length $file_text[$file_count];
    $file_count++;
};

$wasws = 0;
%rnames = ();

print ":userdoc.\n";
print ":title.FTE Manual\n";

for ($pass = 1; $pass <= 2; $pass++) {
    $nheads = 0;
    $list_level = 0;
    $head_level = 0;
    $pre = 0;
    $in_head = 0;
    $ahref = 0;
    $naname = 0;

    @styles = ();

    for ($fn = 0; $fn < $file_count; $fn++) {
        $line = $file_text[$fn];
        $curfile = $file_name[$fn];
        $cpos = 0;
        
        TAG: while ($opos = $cpos,
                    ($cpos = index($line, "<", $cpos) + 1) != 0) # skip to next tag
        {
            pos($line) = $cpos;  # match regexp there
            
            &out(substr($line, $opos, pos($line) - $opos - 1)); # output text
            
            $TAG = undef;
            
            if ($line =~ /\G!(.*?)(--.*?--\s*)*(.*?)>/sgo) { # comment
                $cpos = pos $line;
                #&comment($2);
                next TAG;
            }
            
            pos ($line) = $cpos;
            if ($line =~ /\G\s*([\/]?[A-Za-z0-9]*)/go) {
                $cpos = pos($line);
                $TAG = uc $1;
                
                #print "<|", $TAG, "\n";
            }
            
            undef %ARGS;
            
            ARG:
                while (1) {
                    pos($line) = $cpos;
                    
                    if ($line =~ /\G\s*/go) { $cpos = pos ($line); } # skip whitespace
                    
                    last ARG unless $line =~ /\G([A-Za-z0-9]+)\s*/go; # param name
                    $cpos = pos $line;
                    
                    $pname = uc $1;
                    if ($line =~ /\G=\s*/go) {
                        $cpos = pos $line;
                        
                        if ($line =~ /\G"([^"]*)"\s*/go) {
                            $cpos = pos $line;
                            #print "+|$pname=\"$1\"\n";
                            $ARGS{$pname} = $1;
                            next ARG;
                        };
                        pos($line) = $cpos;
                        if ($line =~ /\G'([^']*)'\s*/go) {
                            $cpos = pos $line;
                            #print "+|$pname='$1'\n";
                            $ARGS{$pname} = $1;
                            next ARG;
                        };
                        pos($line) = $cpos;
                        if ($line =~ /\G([^ <>"']+)\s*/go) {
                            $cpos = pos $line;
                            #print "+|$pname=$1\n";
                            $ARGS{$pname} = $1;
                            next ARG;
                        };
                        $ARGS{$pname} = "";
                        die "no value for tag";
                    }
                    #print "+|$pname\n";
                }
                pos($line) = $cpos;
                ($cpos = index($line, ">", $cpos) + 1) != 0 or die "tag without end";
                
                &tag($TAG, \%ARGS);
        }
        
        &out(substr($line, $opos, length($line) - $opos));
        print STDERR "\n";
    }

    warn "styles left on stack: " . join(">", @styles) if  ($#styles >= 0);
}

print "\n:euserdoc.\n";

print STDERR $nheads . " headings.\n";

sub put {
    my $lin = $_[0];

    print $lin;

    $wasws = ($lin =~ /[\n\t ]$/os);
}

sub pushstyle {
    my $style = $_[0];

    print ":$style.";
    return ;
    $i = $#styles;
    while ($i > 0) {
        print ":e$styles[$i].";
        $i--;
    }
    push(@styles, $style);
    print ":$style.";
}

sub popstyle {
    my $style = $_[0];

    print ":e$style.";
    return ;

    warn if $style ne $styles[$#style];
    print ":e$style.";
    pop (@styles);
    $i = 0;
    while ($i < $#styles) {
        print ":$styles[$i].";
        $i--;
    }
}
sub addindex {
    my $what = $_[0];
    my $id = $_[1];

    $rnames{$what} = $id;
    #print STDERR "$what :: $id\n";
}

sub tag {
    my $TAG = $_[0];
    my %ARGS = %{$_[1]};

    if ($pass == 1) { # during first pass, check for: A NAME=...
        $TAG =~ /^\/H[1-6]$/o  && do {
            $nheads++;
            print STDERR ".";
        }
        or $TAG eq 'A' && do {
            $naname++;
            #print STDERR %ARGS if $naname > 1;
            if (defined $ARGS{"NAME"}) {
                $aname = $curfile . '#' . $ARGS{"NAME"};
                addindex($aname, $nheads - 1);
            } 
        }
        or $TAG eq '/TITLE' && do {
            addindex($curfile, $nheads);
            $nheads++;
        }
        or $TAG eq '/A' && do {
            $naname-- if ($naname > 0);
        }
    } elsif ($pass == 2) {
        $list_level = ($list_level < 0) ? 0 : $list_level;
        
        $TAG eq 'B'       && do { pushstyle('hp2') unless $in_head; }
        or $TAG eq '/B'      && do { popstyle('hp2') unless $in_head; }
        or $TAG eq 'STRONG'  && do { pushstyle('hp7')  unless $in_head; }
        or $TAG eq '/STRONG' && do { popstyle('hp7') unless $in_head; }
        or $TAG eq 'I'       && do { pushstyle('hp1')  unless $in_head; }
        or $TAG eq '/I'      && do { popstyle('hp1') unless $in_head; }
        or $TAG eq 'TT'      && do { pushstyle('hp2')  unless $in_head; }
        or $TAG eq '/TT'     && do { popstyle('hp2') unless $in_head; }
        or $TAG eq 'BR'      && do { put("\n.br\n"); $wasws = 1; }
        or $TAG eq 'HR'      && do { put("\n.br\n"); $wasws = 1; }
        or $TAG eq 'P'       && do { put("\n:p."); $wasws = 1; }
        or $TAG eq 'LI'      && do { put("\n:li."); $wasws = 1;} 
        or $TAG eq 'CENTER'  && do { put(':lines align=center.'); }
        or $TAG eq '/CENTER' && do { put(':elines.'); $wasws = 1; }
        or $TAG eq 'DL'      && do { put(":dl " . $dl_param . '.'); $list_level++; } 
        or $TAG eq '/DL'     && do { put(':edl.'); $list_level--; $wasws = 1; }
        or $TAG eq 'DD'      && do { put("\n:dd."); $wasws = 1; }
        or $TAG eq 'DT'      && do { put("\n:dt."); $wasws = 1; }
        or $TAG eq 'PRE'     && do { put(':xmp.'); $pre++; }
        or $TAG eq '/PRE'    && do { put(':exmp.'); $pre--; $wasws = 1; }
        or $TAG eq 'XMP'     && do { put(':xmp.'); $pre++; }
        or $TAG eq '/XMP'    && do { put(':exmp.'); $pre--; $wasws = 1; }
        or $TAG eq 'OL>'     && do { put(":ol " . $ol_param . '.'); $list_level++; }
        or $TAG eq '/OL'     && do { put(":eol."); $list_level--; $wasws = 1; }
        or $TAG eq 'UL'      && do { put(":ul " . $ul_param . '.'); $list_level++; }
        or $TAG eq '/UL'     && do { put(":eul."); $list_level--; $wasws = 1; }
        or $TAG eq 'IMG'     && do {
            $pic = $ARGS{"SRC"};
            $pic =~ s/gif$/bmp/i;
            put(":artwork runin name='$pic'.") unless $in_head;
        }
        or $TAG eq 'TITLE' && do {
            $hl = 1;
            if ($hl > $head_level + 1) { # hack for bad headings
                $hl = $head_level + 1;
            }
            $head_level = $hl;
            put("\n:h$hl id=$nheads.");
            $in_head = 1;
            $curhead = "";
        }
        or $TAG eq '/TITLE' && do {
            $nheads++;
            put("\n" . $fontspec . $lmargin . ":i1." . $curhead . "\n:p.");
            $in_head = 0;
            $wasws = 1;
        }
        or $TAG eq '/A' && do { 
            if ($ahref > 0) {
                put(":elink.");
                --$ahref;
            }
        }
        or $TAG eq 'A' && do {
            if (defined $ARGS{"HREF"}) {
                $ref = $ARGS{"HREF"};
                $ref = $curfile . $ref if $ref =~ /^\#/;
                
                if (defined $rnames{$ref}) {
                    $id = $rnames{$ref};
                    put(":link reftype=hd refid=$id.");
                    ++$ahref;
                } else {
                    print STDERR "no link for " . $ref . "\n";
                }
                #} else {
                    # print STDERR "external ref not handled: " . $ARGS{"HREF"} . "\n";
                    #}
            }
        }
        or $TAG =~ /^\/H[1-6]$/o  && do {
            $nheads++;
            put("\n" . $fontspec . $lmargin . ":i1." . $curhead . "\n:p.");
            $in_head = 0;
            $wasws = 1;
            print STDERR ".";
        }
        or $TAG =~ /^H([1-6])/o   && do {
            $hl = $1 + 1;
            if ($hl > $head_level + 1) { # hack for bad headings
                $hl = $head_level + 1;
            }
            $head_level = $hl;
            put("\n:h$hl id=$nheads.");
            $in_head = 1;
            $curhead = "";
        }
    }
}

sub out {
    my $lin = $_[0];
    my $first = 1;
    my $i;

    return if ($pass == 1);

    #$lin =~ s/\&lt\;/\</og;
    #$lin =~ s/\&gt\;/\>/og;
    #$lin =~ s/\&amp\;/\&/og;
    ##$lin =~ s/\n/ /og;
    #print $lin;

    #    $lin =~ s/\./\&per\./og;             # .
    $lin =~ s/\&lt\;/\</og;            # <
    $lin =~ s/\&gt\;/\>/og;            # >
    $lin =~ s/\:/\&colon\./og;         # :
    $lin =~ s/\&amp\;/\&amp\./og;      # &

    if ($pre > 0) {
        print $lin; 
    } else {
#        $lin =~ s/\n / /osg;
        $lin =~ s/\n/ /osg;
        $lin =~ s/ +/ /og;
        if ($wasws) {
            $lin =~ s/^ +//o;
        }
        if ($in_head) {
            $curhead .= $lin;
        }
        
        while ($lin ne "") {
            put("\n") unless ($first);
            put(" ") if $line =~ /^\./;
            if (length($lin) <= 70) {
                put($lin);
                $lin = "";
            } else {
                $i = 70;
                if ($i > length $lin) { $i = length $lin; }
                while ($i > 0 && substr($lin, $i, 1) ne ' ') { $i--; }
                if ($i == 0) { $i = 70 };
                if ($i > length $lin) { $i = length $lin; }
                put(substr($lin, 0, $i));
		$lin = substr($lin, $i + 1);
                $lin =~ s/^ +//o;
                $first = 0;
            }
        }
    }
}

sub comment {
#my $comm = $_[0];

#print $comm;
}

sub badtag {
#    my $badtag = $_[0];
#    print "<$badtag>"; # ?
}
__END__
sub tag {
    my $TAG = $_[0];
    my %PARM = %{$_[1]};
    my @ARGS = @{$_[2]};
    
    print "<$TAG";
    foreach $n (@ARGS) {
        $key = $ARGS[$n][2];
        print ' ';
        print $key                             if $ARGS[$n][0] == -1;
        print $key, '=', $ARGS[$n][1]          if $ARGS[$n][0] == 0;
        print $key, '=\'', $ARGS[$n][1], '\''  if $ARGS[$n][0] == 1;
        print $key, '="', $ARGS[$n][1], '"'    if $ARGS[$n][0] == 2;
    }
    print ">\n";
}

sub out {
    my $lin = $_[0];
    
    $lin =~ s/\n/ /;
    
    print " |", $lin, "\n";
}

sub comment {
    my $comm = $_[0];

    print "#|", $comm;
}

__END__
