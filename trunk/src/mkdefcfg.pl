#!perl -w
print "/* do not edit */\nchar DefaultConfig[] = {\n";

$n = 1;
$buf = "";
$out = "";
while (($len = sysread(STDIN, $buf, 256)) > 0) {
    #$buf =~ s/(.)/sprintf "0x%02X", ord($1))/goe;
    #for ($i = 0; $i < $len; $i++) {
    #    $out .= sprintf "0x%02X", ord(substr($buf, $i, 1,));
    #    if ($n++ % 10) {
    #        $out .= ", ";
    #    } else {
    #        $out .= ",\n";
    #    }
    #}

    @c = split(//, $buf);
    for ($i = 0; $i < $len; $i++) {
        $out .= sprintf("0x%02X", ord($c[$i]));
        if ($n++ % 10) {
            $out .= ", ";
        } else {
            $out .= ",\n";
        }
    }

    print $out;
    $out = "";
    print STDERR ".";
}
print "\n};\n";
print STDERR "\n";

