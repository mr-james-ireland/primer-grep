
## a very slow and clunky way to generate a pgrep seq db
## usage: perl generatePgrepDB.pl myseq.fa mydb.pdb

my $SEQ_FILENAME_SZ = 256;

## compress fasta file for jgrep
my %AMBIG = ('M' => ['A','C'],
             'R' => ['A','G'],
             'W' => ['A','T'],
             'S' => ['C','G'],
             'Y' => ['C','T'],
             'K' => ['G','T'],
             'V' => ['A','C','G'],
             'H' => ['A','C','T'],
             'D' => ['A','G','T'],
             'B' => ['C','G','T'],
             'X' => ['A','C','G','T'],
             'N' => ['A','C','G','T']);
             
my %NOTAMBIG = ('A' => 'A',
                'C' => 'C',
                'G' => 'G',
                'T' => 'T');             
              
main();

sub main {

  ## read through the file twice... once
  ## to get size and once to spit out the binary
  ## file
  open(FILE,$ARGV[0]) || die "$!";
  my $curSize = 0;
  my $curName;
  my %seqSizes;
  my %seqNames;
  while (my $line = <FILE>) {
    chop $line; 
    $line =~ s/\s*$//;
    if ($line =~ /^>\s*(.+)/) {
      my $name = substr($1,0,$SEQ_FILENAME_SZ-1);
      $name = $name . (chr(0) x ($SEQ_FILENAME_SZ - length($name)));
      
      $seqNames{$1} = $name;
      if (defined $curName) {
        $seqSizes{$curName} = int(($curSize+3)/4);
      }
      $curSize = 0;      
      $curName = $name;
    } else {
      $line =~ s/[^a-zA-Z]//g;
      $curSize += length($line);
    }
  }
  close FILE;
  
  ## finish up
  $seqSizes{$curName} = int(($curSize+3)/4);
  
  ## open the file a second time and dump 
  open(FILE,$ARGV[0]);
  open(OUT,"> $ARGV[1]");
  binmode OUT;
  my $text="";
  my $outBytes = 0;
  my $expectedSize = 0;
  my $lc=0;
  my $skip = 0;
  while (my $line = <FILE>) {
    chop $line;
    $line =~ s/\s*$//;
    $lc++;
    if ($line =~ /^>\s*(.+)/) {
      $skip = 0;
      if (length($text) > 0) {
        ## dump last bit
        my @textArray = translateText($text);
        map {print OUT chr($_)} @textArray; 
        $outBytes += @textArray+0;
       
      }
      if ($expectedSize != 0 && $expectedSize != $outBytes) {
        print STDERR "ERROR:  Expected size: $expectedSize.  Output: $outBytes\n";
        die;
      }
      my $sn = $1;
      
      print OUT "$seqNames{$sn}";
      print OUT pack("I",$seqSizes{$seqNames{$sn}});
      $expectedSize = $seqSizes{$seqNames{$sn}};
      print STDERR "$seqNames{$sn} - $seqSizes{$seqNames{$sn}}\n";
      print STDERR "LC: $lc\n";
      $outBytes = 0;
      $text = "";
    } else {
      next if ($skip);
      $line =~ s/[^a-zA-Z]//g;
      $text .= $line;
      
      my $tlen = int(length($text)/4);
      my @textArray = translateText(substr($text,0,$tlen*4));
      map {print OUT chr($_)} @textArray; 
      $outBytes += @textArray+0;
      
      $text = substr($text,$tlen*4);
   
    }
  }
  close FILE;

  my @textArray = translateText($text);
  map {print OUT chr($_)} @textArray; 
  close OUT;
}

sub translateText {
  my $text = shift;
  
  my @textArray;
  
  my $i=0;
  
  # pad text if not multiple of 4
  if (length($text) % 4) {
    $text .= "A" x (4-((length($text) % 4)));
  }
  while($i*4 < length($text)) {
    
    $textArray[$i] = char2bits(substr($text,$i*4,4));
    $i++;
  }
  return @textArray;
}
  
sub base2base {
  my $base = shift;
  return $base if (defined $NOTAMBIG{$base});
  
  $base = 'N' unless (defined $AMBIG{$base});
  
  my @posBases = @{$AMBIG{$base}};
  return $posBases[int(rand(@posBases+0))];
}
  
sub char2bits {
  my $chr = uc shift;
  my $bits = 0;
  for (my $j=0; $j < 4; $j++) {
   $bits <<= 2;
    my $c = base2base(substr($chr,$j,1));
    if ($c eq 'T') {
      $bits |= 3;
    } elsif ($c eq 'C') {
      $bits |= 1;
    } elsif ($c eq 'G') {
      $bits |= 2;
    } elsif ($c eq 'A') {
      $bits |= 0;
    } else {
      die "ERROR:  Weird base!  [$c]\n";
    }
   }
  return $bits;
}
    