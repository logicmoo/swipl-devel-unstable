/*  $Id$

    Part of SWI-Prolog

    Author:        Jan Wielemaker
    E-mail:        jan@swi.psy.uva.nl
    WWW:           http://www.swi-prolog.org
    Copyright (C): 1985-2002, University of Amsterdam

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "pl-incl.h"

#undef LD
#define LD LOCAL_LD

static
PRED_IMPL("is_list", 1, is_list, 0)
{ if ( lengthList(A1, FALSE) >= 0 )
    succeed;

  fail;
}


word
pl_length(term_t list, term_t l)
{ GET_LD
  int n;

  if ( PL_get_integer(l, &n) )
  { if ( n >= 0 )
    { term_t h = PL_new_term_ref();
      term_t l = PL_copy_term_ref(list);

      while( n-- > 0 )
      { TRY(PL_unify_list(l, h, l));
      }

      return PL_unify_nil(l);
    }
    fail;
  }

  if ( PL_is_variable(l) )
  { long n;
  
    if ( (n=lengthList(list, FALSE)) >= 0 )
      return PL_unify_integer(l, n);

    fail;			/* both variables: generate in Prolog */
  }
  
  return PL_error("length", 2, NULL, ERR_TYPE, ATOM_integer, l);
}  


word
pl_memberchk(term_t e, term_t list)
{ GET_LD
  term_t h = PL_new_term_ref();
  term_t l = PL_copy_term_ref(list);

  for(;;)
  { TRY(PL_unify_list(l, h, l));
      
    if ( PL_unify(e, h) )
      succeed;
  }
}


static int
qsort_compare_standard(const void *p1, const void *p2)
{ GET_LD
  return compareStandard((Word) p1, (Word) p2 PASS_LD);
}


static term_t
list_to_sorted_array(term_t List, int *size ARG_LD)
{ int n = lengthList(List, TRUE);
  term_t rval;
  term_t list = PL_copy_term_ref(List);
  term_t head = PL_new_term_ref();
  long minfree;
  int i;

  if ( n < 0 )
    fail;				/* not a proper list */

  minfree = (long)sizeof(word)*n;
					/* Won't work anyhow */
  if ( spaceStack(local) < minfree )
  { outOfStack((Stack)&LD->stacks.local, STACK_OVERFLOW_RAISE);
    fail;
  }

#if O_SHIFT_STACKS
					/* grow stack if needed */
  if ( roomStack(local) < minfree )
  { growStacks(NULL, NULL, NULL, minfree, FALSE, FALSE);
  }
#endif

  rval = PL_new_term_refs(n);
  
  for(i=0; PL_get_list(list, head, list); i++)
    PL_put_term(rval+i, head);

  qsort(valTermRef(rval), n, sizeof(word), qsort_compare_standard);
  
  *size = n;
  return rval;
}


word
pl_msort(term_t list, term_t sorted)
{ GET_LD
  term_t array;
  term_t l = PL_copy_term_ref(sorted);
  term_t h = PL_new_term_ref();
  int n, i;

  if ( !(array = list_to_sorted_array(list, &n PASS_LD)) )
    fail;
  for(i=0; i < n; i++)
  { if ( !PL_unify_list(l, h, l) ||
	 !PL_unify(h, array+i) )
      fail;
  }

  return PL_unify_nil(l);
}


word
pl_sort(term_t list, term_t sorted)
{ GET_LD
  term_t array;
  term_t l = PL_copy_term_ref(sorted);
  term_t h = PL_new_term_ref();
  int n, size;
  Word p;

  if ( !(array=list_to_sorted_array(list, &size PASS_LD)) )
    fail;
  p = valTermRef(array);

  for(n = 0; n < size; n++)
  { if ( n == 0 || compareStandard(p+n-1, p+n PASS_LD) != 0 )
    { if ( !PL_unify_list(l, h, l) ||
	   !PL_unify(h, array+n) )
	fail;
    }
  }

  return PL_unify_nil(l);
}

		 /*******************************
		 *      PUBLISH PREDICATES	*
		 *******************************/

BeginPredDefs(list)
  PRED_DEF("is_list", 1, is_list, 0)
EndPredDefs
