#!/usr/bin/env perl

#------------------------------------------------------------------------------
# z80asm - build grammar file
# Copyright (C) Paulo Custodio, 2011-2024
# License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
#------------------------------------------------------------------------------

use Modern::Perl;
use Path::Tiny;
use YAML::Tiny;
use List::Util 'max';

@ARGV==2 or die "Usage: $0 input_file.yaml output_file.h\n";
my($input_file, $output_file) = @ARGV;

my $yaml = YAML::Tiny->read($input_file);
my %opcodes = %{$yaml->[0]};

my %cpus = read_cpus('../../../common/z80asm_defs.h');		# cpu => id

my %tokens = read_tokens('../scan.def');					# text => id
my $nr_tokens = max(values %tokens) +1 ;
my $base_tokens = 0;

my %keywords = read_keywords('../scan.def');				# text => id
my $nr_keywords = max(values %keywords) + 1;
my $base_keywords = $nr_tokens;

my $base_expr = $nr_tokens+$nr_keywords;

# convert opcodes into trie tree
my $tree = { state=>0, next=>{}, current=>"top" };
my @states = ($tree);
my @actions = ('');		# actions start at 1
my @state_comment = ('');
my %actions;

for my $asm (sort keys %opcodes) {
	my $cur_state = 0;
	my $asm1 = $asm;
	while ($asm1 ne "") {
		$asm1 =~ s/^\s+//;
		last if $asm1 eq "";
		
		if ($asm1 =~ s/^(%[a-z])//i) {
			$column = $base_expr;
		}
		elsif (($column = consume_token(\$asm1, %keywords)) != 0) {
			$column += $base_keywords;
		}
		elsif (($column = consume_token(\$asm1, %tokens)) != 0) {
			$column += $base_tokens;
		}
		else {
			die "cannot parse: $asm:$asm1\n";
		}

		my $next_state = $parse_table[$cur_state][$column];
		if (!defined $next_state) {
			$next_state = scalar(@parse_table);
			push @parse_table, ([]);
			$parse_table[$cur_state][$column] = $next_state;
		}
		
		$cur_state = $next_state;
	}
}

use Data::Dump 'dump';
dump \@parse_table;


#------------------------------------------------------------------------------
sub read_cpus {
	my($file) = @_;
	my %cpus;
	for (path($file)->lines) {
		if (/^ \s* CPU_(\w+) \s* = \s* (\d+) /x) {
			next if $1 eq 'UNDEF';
			$cpus{lc($1)} = $2;
		}
	}
	return %cpus;
}

#------------------------------------------------------------------------------
sub read_tokens {
	my($file) = @_;
	my %tokens;
	my $id = 0;
	for (path($file)->lines) {
		if (/^ \s* TK\( \s* (TK_\w+) \s* , \s* \" ([^\"]*) \" /x) {
			my($const, $string) = ($1, $2);
			$string =~ s/\\\\/\\/g;
			$tokens{$string} = $id unless $string eq "";
			$id++;
		}
	}
	return %tokens;
}

#------------------------------------------------------------------------------
sub read_keywords {
	my($file) = @_;
	my %keywords;
	my $id = 0;
	for (path($file)->lines) {
		if (/^ \s* KW\( \s* (KW_\w+) \s* , \s* \" ([^\"]*) \" /x) {
			my($const, $string) = ($1, $2);
			$keywords{$string} = $id unless $string eq "";
			$id++;
			if ($string ne "") {
				$string =~ s/'/1/;
				die $_ unless $const eq "KW_".uc($string);
			}
		}
	}
	return %keywords;
}

#------------------------------------------------------------------------------
sub consume_token {
	my($textref, %tokens) = @_;
	my $column = 0;
	my $found = "";
	while (my($text, $id) = each %tokens) {
		if (substr($$textref, 0, length($text)) eq $text) {
			if (length($text) > length($found)) {
				$found = $text;
				$column = $id;
			}
		}
	}
	
	$$textref = substr($$textref, length($found));
	return $column;
}
