#!perl -w

print <<EOD;
<HTML>
<HEAD>
<TITLE>Macro Commands</TITLE>
</HEAD>
<BODY>
EOD

sub out {
    my $lin = $_[0];
    $lin =~ s/\&/\&amp./og;
    $lin =~ s/\</\&lt;/og;
    $lin =~ s/\>/\&gt;/og;
    return $lin;
}

sub out_seealso {
    my $first = 1;
    my $section = $_[0];
    
    print "<P><B>SEE ALSO:</B>\n";
    if ($#see_also >= 0) {
        foreach $see (@see_also) {
            if (!$first) { print ", "; }
            print $see;
            $first = 0;
        }
    }
    if (defined $section) {
        if (!$first) { print ", "; }
        print qq|<A HREF="#$sec_ref{$section}">| . out($section) . "</A>";
        $first = 0;
    } else {
        foreach (@sections) {
            if (!$first) { print ", "; }
            print qq|<A HREF="#$sec_ref{$_}">| . out($_) . "</A>";
            $first = 0;
        }
    }
    if (!$first) { print "."; }
    @see_also = ();
}

sub out_contents {
    my $section = $_[0];

    print "<UL>\n";
    foreach (@{$contents{$section}}) {
        print qq|<LI><A HREF="#ec.$_">| . $_ . "</A>\n";
    }
    print "</UL>\n";
}

sub out_sections {
    print "<UL>\n";
    foreach (@sections) {
        print qq|<LI><A HREF="#$sec_ref{$_}">| . $_ . "</A>\n";
    }
    print "</UL>\n";
}

sub new_command {
    if ($newcommand) {
        if ($output) {
            if ($was_section) {
                out_seealso();
            } else {
                out_seealso($section);
            }
            print qq|<HR><H2><A NAME="ec.$command">| . out($command) . "</A></H2>\n";
        } else {
            push(@{$contents{$section}}, $command);
        }
        $was_command = 1;
        $newcommand = 0;
        $was_section = 0;
    }
}

@file = <>;

for ($output = 0; $output <= 1; $output++) {
    out_sections() if $output;
    @see_also = ();
    $was_command = 0;
    foreach (@file) {
        if (m|Ex([A-Za-z]+)\,|) {
            $command = $1;
            $newcommand = 1;
        } elsif (m|///\s*(.*)|) {
            $text = $1;
            new_command();
            if ($output) {
                if ($text eq '') {
                    print '<P>' unless $was_command || $was_section;
                } else {
                    print out($text), "\n";
                }
            }
        } elsif (m|//\\\s*(.*)|) {
            $text = $1;
            new_command();
            if ($output) {
                if ($text eq '') {
                    print '<BR>' unless $was_command || $was_section;
                } else {
                    print $text, "\n";
                }
            }
        } elsif (m|//\&\s*(.*)|) {
            push (@see_also, $1);
        } elsif (m|//<([^>]*)>\s*(.*)|) {
            out_seealso($section) if $was_command && $output;
            $ref = $1;
            $section = $2;
            if ($output) {
                print qq|<HR><H1><A NAME="$ref">| . out($section) . "</A></H1>\n";
                out_contents($section);
            } else {
                $contents{$section} = [];
                $sec_ref{$section} = $ref;
                #$ref_sec{$ref} = $section;
                push(@sections, $section);
            }
            $newcommand = 0;
            $was_section = 1;
            $was_command = 0;
        }
    }
}

out_seealso($section);

print <<EOD;
</BODY>
</HTML>
EOD
