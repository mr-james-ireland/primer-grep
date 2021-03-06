
PGREP v1.1 - README
Author: James Ireland, jireland@alternateallele.com

-------------------------------
Copyright (C) 2007 Affymetrix, Inc.                                         
                                                                             
This program is free software; you can redistribute it and/or modify        
it under the terms of the GNU General Public License (version 2) as         
published by the Free Software Foundation.                                  
                                                                            
This program is distributed in the hope that it will be useful,             
but WITHOUT ANY WARRANTY; without even the implied warranty of              
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU            
General Public License for more details.                                    
                                                                            
You should have received a copy of the GNU General Public License           
along with this program;if not, write to the                                
                                                                            
Free Software Foundation, Inc.,                                             
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                       

Table of Contents
------------------------------
1.  Installation
2.  Quick Start
3.  Options
4.  Search Options
4.1 Filter Search
4.2 Full Search
4.3 Mismatch Weights
5.  Input/Output formats
6.  Known Issues

1. Installation
------------------------------
A.  Pre-compiled binaries

For Windows XP and a handfull of other platforms, pgrep comes precompiled.  After unzipping pgrep.1.1.zip
or untaring/zipping pgrep.1.1.tar.gz, go to the bin directory and find the platform that's right for you.

B.  Building the application

Linux/Unix

Unzip, untar and build pgrep as follows:

$ tar -xzf pgrep.1.1.tar.gz
$ cd pgrep1.1/src
$ make

If you do not have gnu g++ installed then edit the Makefile with the appropriate compiler and
compilation flags.  

To test that pgrep has compiled correctly, run the tests.

$ cd ../tests
$ make test

All tests should say that they passed.

C.  Creating a sequence database.

PGREP searches primers against a precompiled database.  The easiest way to create the necessary sequence
database is with the BLAST application formatdb.  PGREP has been tested against databases compiled with
formatdb from version 2.2.8 of the NCBI BLAST suite.  If you don't have the BLAST suite already, you might
try finding it here: http://www.ncbi.nlm.nih.gov/blast/download.shtml

Create the database as follows:

$ formatdb -p F -i myseqs.fa

This will create three database files.  PGREP requires only the *.nin and *.nhr files for its searches.

Alternatively, you may use the included Perl script to compile sequence databases.  Note that this script
is much slower than formatdb.  To use the script:

$ perl generatePgrepDB.pl myseqs.fa myseqs.pdb

Note: when running pgrep, make sure to use the -b flag when search against a BLAST database or -d if you
are searching against one compiled with the perl script.

D.  Running on Windows

PGREP must be run from the MS-DOS command line on Windows machines.  

2. Quick Start
------------------------------
A.  Filter candidate primers directly from primer3 and pass only the primer3 results from the first passing
pair:

$ primer3 < primer3_input.bldr | pgrep -b sequencedb.fa -k 2 -f 4 -F 2

Here a primer passes only if it matches one site with 2 or fewer mismatches with an amplicon size
of 4 times smaller or 2 times larger than the expected size.

B.  Filter a tab-delimited file of candidate primers and return the id of the first passing pair:

$ pgrep -b sequencedb.fa -p primers.txt -k 2 -f 4 -F 2

Here a primer passes only if it matches one site with 2 or fewer mismatches with an amplicon size
of 4 times smaller or 2 times larger than the expected size.

C.  Return all matches for a primer pair supplied on the command line:

$ prep -b sequencedb.fa -l aatctgatgagcctccttttt -r tccttcccctcagacactt -k 2 -a 100 -A 500 -M 1000 -o

A match here has 2 or fewer mismatches in the primers with an amplicon size between 100-500bp long.
Up to 1000 hits are displayed in "full output" mode.

D.  Find all matches to the genome of a tab-delimited file of primers using a custom mismatch weighting:

$ pgrep -b sequencedb.fa -p primers.txt -w weights.txt -k 2 -f 4 -F 2 -M 1000 -o

3.  Options
-------------------------------

	-a MIN_AMP_SIZE
		Minimum amplicon size searched for when finding potential priming sites.
		
	-A MAX_AMP_SIZE
		Maximum amplicon size searched for when finding potential priming sites.

	-b BLAST_DB1 BLAST_DB2 ...
		Blast databases to search.  If BLAST_DB is listed, then PGREP expects to find files
		BLAST_DB.nin, BLAST_DB.nhr.  Specify either PGREP or Blast databases.
		
	-c DB_FILE
		When searching through multiple sequence database files (such as multiple blast database files),
		start the search with this file.  This can speed up primer screening somewhat if the database
		file where the true priming site is expected is searched first.  So, if primers were designed for
		a gene on chromosome 12, the chromosome 12 blast database should be given here.

	-C 
		Do not reverse complement the reverse primer when searching.
		
	-d PGREP_DB1 PGREP_DB2 ...
		PGREP formatted databases to search.  Specify either PGREP or Blast databases.
		
	-f MIN_AMP_FACTOR
		Search for amplicons that are at least as big as 1/MIN_AMP_FACTOR of the expected amplicon size.
		
	-F MAX_AMP_FACTOR
		Search for amplicons that are at most as big as MAX_AMP_FACTOR-times the size of the expected amplicon size.
		
	-h
		Help.  Brief description of usage.
		
	-k MAX_MISMATCH_SCORE
		Search for priming sites where the weighted match for each primer has MAX_MISMATCH_SCORE or less.  For
		example, using the default weighting (all mismatches get a penalty of one), then specifying a k value
		of 2 would find matches where one or both primers had 2 or fewer mismatches.  Therefore, the combined
		mismatch score of the two primers could be as much as 4.
		
	-l LEFT_PRIMER_SEQUENCE
		Use this option to quickly search for a single primer pair specified on the command line.  A right primer
		must also be specified as well as absolute amplicon size limits (options -a and -A).
		
	-L MAX_PRIMERS_RETURNED
		Return all passing primers up to MAX_PRIMERS_RETURNED.  Default behavior is just to return the first 
		passing primer pair.
		
	-m MIN_ALLOWED_NUMBER_HITS
		Minimum number of priming sites that must exist for a primer pair to pass.  The default is 1, which
		just means that the primers must match the database at least somewhere.

	-M MAX_ALLOWED_NUMBER_HITS
		Maximum number of priming sites that may exist for a primer pair to still pass.  The default is 1, which
		means that the primers must match the database at one location uniquely.  When using the -o option to
		get a more detailed list of all the priming sites you may wish to increase this option to return all of
		the hit locations.

	-o 
		Full output mode.  Returns potential amplification sites in a tab-delimited format.
		
	-p PRIMER_FILE
		Tab-delimited primer file.  The first line of the file is assumed to be a header line.  Subsequent
		lines should follow the format:
		<primer pair id><tab><forward/left primer><tab><reverse/right primer><tab><amp size>
		Other columns may exist beyond these four, but they will be ignored.
		
	-r RIGHT_PRIMER_SEQUENCE
		Use this option to quickly search for a single primer pair specified on the command line.  A left primer
		must also be specified as well as absolute amplicon size limits (options -a and -A).
		
	-w WEIGHT_FILE
		Specifies a mismatch weighting file.  See details below.
			
4.  Search Options
-------------------------------
PGREP supports two types of search modes: filter and full search.  Filter search takes a set of candidate
primers and returns the first N pairs which satisfies the search conditions.  By default, N=1, but this can
be increased through the -L option.  Primers will be filtered if they match too many (or possibly too few)
sites in the sequence database.  The expected usage is that a list of primers for one target will be generated
by a primer design program like Primer3 ranked from best to worst based on properties like TM, self-annealing,
etc.  PGREP is then used to pick the best pair based on amplification specificity.

PGREP also supports full search mode where one or more primer pair is searched against the sequence database
and all matches are returned.  This mode is more like a general sequence homology search like BLAST, BLAT or 
ePCR.  PGREP is much slower than these applications and it is generally recommended to use one of these tools if
they meet your needs.  Where you might want to use PGREP is if you need increased sensitivity (most hash-
based searches like those listed can miss some hits if there is not a good "seed" match between primer and
template) or if you wish to search with a custom weighting function.

4.1  Filter Search
-------------------------------
The primary purpose for writing PGREP was to more seemlessly combine primer design and specificity testing.  Our
approach was to design many candidate primer designs for the targeted region using a primer design tool like 
Primer3.  This list is presumed to be sorted from the best to worst candidate pair.  This list is fed through
PGREP either as a tab-delimited file of primers or directly piped through standard input.  In filter mode, PGREP
will toss out any primer pair that matches the template sequence more than the allowed number of times (typically
one) with the allowed number (or fewer) mismatches between primer and template.  PGREP stops with the first
primer pair that is not filtered (i.e. that is specific) unless more passing primer pairs are requested using the
-L option.  Output from the filter is brief - only the primer id of the passing primers are returned or a 
message stating that no passing primers were found.

4.2  Full Search
-------------------------------
Full search mode is triggered with the -o option and is useful for characterizing potential priming sites.  For
each primer pair given as input, up to -M priming sites which match the amplicon size and mismatch score criteria
will be returned in a tab delimited format.  The output from this search includes the location of the amplification
site, the amplicon size and sequence and primer-template alignments. 

4.3 Mismatch Weights
-------------------------------
A custom weighting scheme may be used for scoring primer-template matches.  Weights in PGREP are penalties
generally associated with primer-template mismatches.  To specify your own weighting scheme, you must supply a
tab delimited file with the following format:
<header row>
<primer (F/R/*)>	<position relative to 5' or 3' (5/3)>	<position start (or *)>	<position stop>	<weight (pos int)>...
...<primer base (A/C/G/T/N/K/etc/*/!/=>)>	<temp base (A/C/G/T/N/K/etc/*/!/=>)>
...more weight lines...

Some examples will be illuminating.  

Here's the default weighting PGREP uses when no custom weighting is specified:

Primer	End	Start	Stop	Weight	PBase	TBase
*		5	*		*		1		N		!

This specifies that for any base in the primer template pairing a penalty of 1 is given where the template
and primer differ.

To increase the penalty to 2 when the mismatch appears within 4 bases of the 3' end, the weighting file would
look like:

Primer	End	Start	Stop	Weight	PBase	TBase
*		5	*		*		1		N		!
*		3	1		4		1		N		!

Note that weights are summed together when multiple weighting rules apply.

Other things to note:
- IUPAC codes may be used for specifiying primer and template bases.  IUPAC code 'N' and '*' are equivalent.
- "!" means that the base is different than the primer (or template) base
- "=" means that the primer and template bases are the same (hard to imagine a scenario when
  you'd want to penalize for matching bases... but the option is there for you!)
- Weights must be positive and integer.  PGREP can currently only look for matches with a score of 15 or
  less, so it is pointless to use weights larger than 15.
- Generally it is good to start your weighting function with a general mismatch penalty like the examples
  above.  If your weighting score does not penalize enough for mismatches you'll get too many hits and PGREP
  will likeley die.
  
5.  Input/Output Formats    
-------------------------------

5.1 Input
-------------------------------

Primers file.  

This is the input file used with the -p option.  The format is:

<header line>
<primer id><tab><left/forward primer><tab><right/reverse primer><tab><amp size>

Other columns may be present in the file, but the first four columns must follow this layout.

Primer3 input.

This format follows the "boulder" format.  Details of this format can be found in the Primer3 documentation.  
Primer3 input files must be provided to pgrep through STDIN.

5.2 Output
-------------------------------

Default mode.

In default mode, pgrep will return to STDOUT one line for each passing primer pair in the format:

Primer pair: <0-based count of primers in the input> is OK.  Name: [<primer pair id>].

or

No good primers found.

when no passing primer pairs are found.

Primer3 output.

When input is provided through STDIN in Primer3 format and full results are not requested (-o), then
the output returned will also be in Primer3 format.

Full results output.

In the case that full results are requested, the format will be as follows:
<the header line>
PrimerPairId	TemplateSequenceName	MatchStart	MatchStop	MatchStrand	MatchLength	ForwardPrimer	ForwardPrimerAlignment	ForwardTemplateSeq	ForwardMismatchScore	ReversePrimer	ReversePrimerAlignment	ReverseTemplateSeq	ReverseMismatchScore	FullMatchSequence
<tab delimited results, e.g.>
test	gi|82653974|gb|CY005231.1| Influenza A virus and primers	813	1083	0	271	AATCTGATGAGCCTCCTTTTT	|||||||||||||| ||||||	AATCTGATGAGCCTACTTTTT	1	TCCTTCCCCTCAGACACTT	|||||||||||||||||||	TCCTTCCCCTCAGACACTT	0	AATCTGATGAGCCTACTTTTTGGAAGTGAGAAATGATGATGTTGACCAGAGCTTGATTATCGCTGCCAGGAACATAGTAAGAAGAGCAACAAATCTGATGAGCCTCCTTTTTGTATCAGCAGACCCACTAGCATCTCTATTGGAGATGTGCCACAGCACGCAAATTGGGGGAATAAGAATGGTAGACATTCTTCGGCAAAATCCAACAGAGGAACAAGCCGTGGACATATGCAAGGCAGCAATGGGCTTAAGAAGTGTCTGAGGGGAAGGA
test	gi|82653974|gb|CY005231.1| Influenza A virus and primers	904	1083	0	180	AATCTGATGAGCCTCCTTTTT	|||||||||||||||||||||	AATCTGATGAGCCTCCTTTTT	0	TCCTTCCCCTCAGACACTT	|||||||||||||||||||	TCCTTCCCCTCAGACACTT	0	AATCTGATGAGCCTCCTTTTTGTATCAGCAGACCCACTAGCATCTCTATTGGAGATGTGCCACAGCACGCAAATTGGGGGAATAAGAATGGTAGACATTCTTCGGCAAAATCCAACAGAGGAACAAGCCGTGGACATATGCAAGGCAGCAATGGGCTTAAGAAGTGTCTGAGGGGAAGGA
   
6.  Known Issues
-------------------------------
These are some known issues or shortcomings of PGREP:
- No indel support.  Currently PGREP will not find matches which include insertions or deletions within the
primer-template match.
- Primers longer than MAXPRIMERSIZE (29 bases) are truncated at the 5' end.
- Limited ambiguous base support in template sequence.  If an ambiguous base is present in the template sequence,
then an unambiguous base from the potential bases is chosen at random.  This is also how BLAST encodes for ambiguous
bases.  Unlike BLAST, however, no attempt is made after primer-template matches are found to identify these sites
as ambiguous.  Instead, the randomly chosen unambiguous base is reported.
- Hits overflow.  It is possible to make PGREP die a silent but painful death if too many potential hits exist.  In
general, I try to keep the number of allowed mismatches at four or fewer for default mismatch weighting with typical
primers against the human genome.
- No matching is performed for potential forward-forward or reverse-reverse primer amplicons.

Some of these issues we hope to address in the near future.  We welcome other suggestions, comments, requests.  
