#------------------------------------------------------------------------------
# z80asm - parser
# Copyright (C) Paulo Custodio, 2011-2024
# License: The Artistic License 2.0, http://www.perlfoundation.org/artistic_license_2_0
#------------------------------------------------------------------------------

"nop" $
	object_->add_instr(0x00)

"ld" "a" "," expr $
	if (exprs.back().in_parens()) {
		auto instr = object_->add_instr(0x3a);
		instr->add_patch(RANGE_WORD, exprs.back());
	}
	else {
		auto instr = object_->add_instr(0x3e);	
		instr->add_patch(RANGE_BYTE_UNSIGNED, exprs.back());
	}

"ld" "a" "," "a" $
	object_->add_instr(0x7f);
