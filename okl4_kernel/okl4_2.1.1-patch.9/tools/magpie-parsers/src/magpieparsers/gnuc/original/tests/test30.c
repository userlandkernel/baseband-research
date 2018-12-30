typedef struct bitmap_element_def
{
  struct bitmap_element_def *next;               
  struct bitmap_element_def *prev;               
  unsigned int indx;                     
  unsigned int  bits[2 ];  
} bitmap_element;

 
typedef struct bitmap_head_def {
  bitmap_element *first;         
  bitmap_element *current;       
  int indx;                      
} bitmap_head, *bitmap;

static __inline__ void
bitmap_element_free (head, elt)
     bitmap head;
     bitmap_element *elt;
{
  bitmap_element *next = elt->next;
}

main() {
      /* See if this is a branch that is part of the path.  If so, and it is
         to be taken, do so.  */
      if (next_branch->branch == insn)
        {
          enum taken status = next_branch++->status;
          if (status != NOT_TAKEN)
            {
              if (status == TAKEN)
                record_jump_equiv (insn, 1);
              else
                invalidate_skipped_block (NEXT_INSN (insn));

              /* Set the last insn as the jump insn; it doesn't affect cc0.
                 Then follow this branch.  */
              prev_insn_cc0 = 0;
              prev_insn = insn;
              insn = JUMP_LABEL (insn);
              continue;
            }
        }
}

