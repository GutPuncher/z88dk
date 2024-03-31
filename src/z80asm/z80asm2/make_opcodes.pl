#!/usr/bin/env perl
#------------------------------------------------------------------------------
# z80asm - make parser_opcodes.y with all opcodes of all cpus
# Copyright (C) Paulo Custodio, 2011-2024
# License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
#------------------------------------------------------------------------------

my $OUTPUT = "parser_opcodes.y";
open(my $fh, ">", "$OUTPUT~") or die "open $OUTPUT~: $!\n";

say $fh <<END;
nop
	object_->add_instr(0x00);

END

for my $n (0..3) {
	my $dd = (qw(bc de hl sp))[$n];
	say $fh <<END;
ld $dd, EXPR
	if (exprs.back().in_parens() && $n==2) {
		auto instr = object_->add_instr(0x2A);
		instr->add_patch(RANGE_WORD, exprs.back());
	}
	else if (exprs.back().in_parens()) {
		auto instr = object_->add_instr(0xed4b+16*$n);
		instr->add_patch(RANGE_WORD, exprs.back());
	}
	else {
		auto instr = object_->add_instr(0x01+16*$n);	
		instr->add_patch(RANGE_WORD, exprs.back());
	}

END
}