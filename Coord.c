#include "Global.h"
#include "Rat.h"

#undef TEST_Wbase
#undef USE_Old_Wbase
#define NO_COORD_IMPROVEMENT /* switch off weight permutation */

typedef struct {
  Long x[AMBI_Dmax][AMBI_Dmax];
  int n, N;
} CWLatticeBasis;

void Make_CWS_Points(CWS *_C, PolyPointList *_P);
void Make_RGC_Points(CWS *Cin, PolyPointList *_P);
void CWS_to_PermCWS(CWS *Cin, CWS *C, int *pi);

/*  ==========  	  I/O functions:                	==========  */

int IsNextDigit(void) {
  char c;
  c = fgetc(inFILE);
  ungetc(c, inFILE);
  if (c == '0')
    return -1;
  if ((c < '0') || ('9' < c))
    return 0;
  else
    return 1;
}

void Print_PPL(PolyPointList *_P, const char *comment) {
  int i, j;
  if (_P->np > 20) {
    fprintf(outFILE, "%d %d  %s\n", _P->np, _P->n, comment);
    for (i = 0; i < _P->np; i++) {
      for (j = 0; j < _P->n; j++)
        fprintf(outFILE, "%d ", (int)_P->x[i][j]);
      fprintf(outFILE, "\n");
    }
  } else {
    fprintf(outFILE, "%d %d  %s\n", _P->n, _P->np, comment);
    for (i = 0; i < _P->n; i++) {
      for (j = 0; j < _P->np; j++)
        fprintf(outFILE, " %4d", (int)_P->x[j][i]);
      fprintf(outFILE, "\n");
    }
  }
}

void Print_VL(PolyPointList *_P, VertexNumList *_V, const char *comment) {
  int i, j;
  if (_V->nv > 20) {
    fprintf(outFILE, "%d %d  %s\n", _V->nv, _P->n, comment);
    for (i = 0; i < _V->nv; i++) {
      for (j = 0; j < _P->n; j++)
        fprintf(outFILE, "%d ", (int)_P->x[_V->v[i]][j]);
      fprintf(outFILE, "\n");
    }
  } else {
    fprintf(outFILE, "%d %d  %s\n", _P->n, _V->nv, comment);
    for (i = 0; i < _P->n; i++) {
      for (j = 0; j < _V->nv; j++)
        fprintf(outFILE, " %4d", (int)_P->x[_V->v[j]][i]);
      fprintf(outFILE, "\n");
    }
  }
}

void Print_EL(EqList *_E, int *n, int suppress_c, const char *comment) {
  int i, j;
  fprintf(outFILE, "%d %d  %s\n", _E->ne, *n, comment);
  for (i = 0; i < _E->ne; i++) {
    for (j = 0; j < *n; j++)
      fprintf(outFILE, " %3d", (int)_E->e[i].a[j]);
    if (!suppress_c)
      fprintf(outFILE, " %5d", (int)_E->e[i].c);
    fprintf(outFILE, "\n");
  }
}

/* Fast integer-to-string: write decimal to buf, return pointer past last char */
static char *itoa_fast(char *buf, int v) {
  char tmp[12];
  int neg = 0, len = 0;
  if (v < 0) { neg = 1; v = -v; }
  do { tmp[len++] = '0' + (v % 10); v /= 10; } while (v);
  if (neg) *buf++ = '-';
  while (len--) *buf++ = tmp[len];
  return buf;
}

void Print_Matrix(Long Matrix[][VERT_Nmax], int n_lines, int n_columns,
                  const char *comment) {
  /* Compact single-line output: [[r0c0,r0c1,...],[r1c0,...],...]  */
  /* Buffer sized for worst case: 12 chars/number, commas, brackets, newline */
  static char buf[VERT_Nmax * POLY_Dmax * 14 + POLY_Dmax * 4 + 8];
  char *p = buf;
  int i, j;
  (void)comment;
  *p++ = '[';
  for (i = 0; i < n_lines; i++) {
    if (i) *p++ = ',';
    *p++ = '[';
    for (j = 0; j < n_columns; j++) {
      if (j) *p++ = ',';
      p = itoa_fast(p, (int)Matrix[i][j]);
    }
    *p++ = ']';
  }
  *p++ = ']';
  *p++ = '\n';
  fwrite(buf, 1, (size_t)(p - buf), outFILE);
}
int auxString2Int(char *c, int *n) {
  int j = 0;
  *n = 0;
  while (('0' <= c[j]) && (c[j] <= '9'))
    *n = 10 * (*n) + c[j++] - '0';
  if (j)
    while (c[j] == ' ')
      j++;
  return j;
}
void CWSZerror(char *c) {
  printf("Format error %s in Read_CWS_Zinfo\n", c);
  exit(0);
}
void Print_CWS_Zinfo(CWS *CW) {
  int i, j;
  if (CW->nw)
    for (i = 0; i < CW->nz; i++) {
      fprintf(outFILE, "/Z%d: ", CW->m[i]);
      for (j = 0; j < CW->N; j++)
        fprintf(outFILE, "%d ", CW->z[i][j]);
    }
}
int Read_CWS_Zinfo(FILE *inFILE, CWS *CW) /* return !EOF */
{
  int *nz = &CW->nz;
  int i = 0, n;
  char c[999], b = ' ';
  *nz = 0;
  for (n = 0; n < 999; n++) {
    c[n] = fgetc(inFILE);
    if (feof(inFILE))
      return 0;
    if (c[n] == '\n')
      break;
  }
  if (n == 999) {
    puts("Out of space in Read_CWS_Zinfo");
    exit(0);
  }
  while (c[i] == b)
    i++;
  if ((c[i] == '=') && (c[i + 1] == 'd'))
    i += 2;
  while (c[i] == b)
    i++;
  /*	printf("Read_CWS_Zinfo n=%d -> ",n);
     {int I; for(I=0;I<n;I++)printf("%c",c[I]);puts("");} */
  while (i < n) {
    int j, k, s;
    if ((c[i] != '/') || (c[i + 1] != 'Z'))
      return 1;
    i += 2;
    assert(*nz < POLY_Dmax);
    if ((j = auxString2Int(&c[i], &CW->m[*nz]))) {
      if (c[i + j] != ':')
        CWSZerror(":");
      else {
        i += j + 1;
        while (c[i] == b)
          i++;
      }
      for (k = 0; k < CW->N; k++) {
        if ((j = auxString2Int(&c[i], &CW->z[*nz][k]))) {
          if ((j))
            i += j;
          else
            CWSZerror("missing");
        } /* ????? */
      }
      s = 0;
      for (k = 0; k < CW->N; k++)
        s += CW->z[*nz][k];
      /*	if(s % CW->m[*nz])  CWSZerror("det!=1"); */
      (CW->nz)++;
    } else
      CWSZerror("Order");
  }
  return 1;
}

void checkDimension(int polyDim, int codim, int index) {
  if (index == 1) {
    if (POLY_Dmax < (polyDim + codim - 1)) {
      if (codim > 1) {
        printf("Please increase POLY_Dmax to at least %d = %d + %d - 1\n",
               (polyDim + codim - 1), polyDim, codim);
        printf("(POLY_Dmax >= dim N + codim - 1 is required)\n");
      } else
        printf("Please increase POLY_Dmax to at least %d\n", polyDim);
      exit(0);
    }
  } else if (POLY_Dmax < polyDim + 1) {
    printf("Please increase POLY_Dmax to at least %d = %d + 1\n", (polyDim + 1),
           polyDim);
    printf("(option -G requires POLY_Dmax >= dim(cone) = dim(support) + 1)\n");
    exit(0);
  }
}

int ReadCwsPp(CWS *_CW, PolyPointList *_P, int codim, int index)
/*   _P is always an M-lattice polytope
     codim = 1, index = 1: CY hypersurface
     codim > 1, index = 1: _P reflexive, CICY with codimension codim
     codim = 1, index > 1: _P a Gorenstein polytope with index index,
                           determines a reflexive Gorenstein cone       */
{
  int i, j, FilterFlag = (inFILE == NULL);
  int IN[AMBI_Dmax * (AMBI_Dmax + 1)], S;
  static int InputOK;
  _CW->nw = _CW->N = _CW->nz = 0;
  _CW->index = index;
  if (FilterFlag)
    inFILE = stdin;
  else if (inFILE == stdin) {
    puts("Degrees and weights  `d1 w11 w12 ... d2 w21 w22 ...'");
    puts("  or `#lines #columns' (= `PolyDim #Points' or `#Points PolyDim'):");
  }
  for (i = 0; i < AMBI_Dmax * (AMBI_Dmax + 1); i++) {
    char c;
    while (' ' == (c = fgetc(inFILE)))
      ;
    ungetc(c, inFILE); /* read blanks */
    if (IsNextDigit())
      fscanf(inFILE, "%d", &IN[i]);
    else
      break;
  }
  if (i == 0) {
    if (!InputOK) {
      puts("-h gives you help\n");
      exit(0);
    } else
      return 0;
  }
  InputOK++;
  if (i == 1) {
    puts("Error in INPUT: need at least 2 numbers!");
    exit(0);
  }
  if (i == 2) /* READ PolyPointList */
  {
    int tr = 0;
    while ('\n' != fgetc(inFILE))
      ; /* read to end of line */
    if (IN[0] == IN[1]) {
      puts("The number of points must be larger than the dimension!");
      exit(0);
    }
    if (IN[0] > IN[1]) {
      tr = IN[0];
      IN[0] = IN[1];
      IN[1] = tr;
    }
    tr = !tr;
    checkDimension(IN[0], codim, index);
    if (IN[1] > POINT_Nmax) {
      puts("Please increase POINT_Nmax");
      exit(0);
    }
    if ((inFILE == stdin) && !FilterFlag)
      printf("Type the %d coordinates as %s=%d lines with %s=%d columns:\n",
             IN[0] * IN[1], tr ? "dim" : "#pts", tr ? IN[0] : IN[1],
             tr ? "#pts" : "dim", tr ? IN[1] : IN[0]);
    _P->n = IN[0];
    _P->np = IN[1];
    /* allow all numbers in one string (distributed over lines) or
     * matrix blocks with trailing comments	*/
    if (tr)
      for (i = 0; i < IN[0]; i++)
        for (j = 0; j < IN[1]; j++) {
          int X;
          fscanf(inFILE, "%d", &X);
          _P->x[j][i] = X;
        }
    else
      for (i = 0; i < IN[1]; i++)
        for (j = 0; j < IN[0]; j++) {
          int X;
          fscanf(inFILE, "%d", &X);
          _P->x[i][j] = X;
        }
    /* Finish_Poly_Points(_P); */
    while (fgetc(inFILE) - '\n')
      if (feof(inFILE))
        return 0; /* read to EOL */
    if (FilterFlag)
      inFILE = NULL;
    return 1;
  } /* End of reading PolyPointList */
  assert(i != 3);
  S = IN[i - 1];
  for (j = 0; j < i - 1; j++)
    if ((IN[j] == 0) || (S < IN[j]))
      break;
  if (j == i - 1) /* Single Weights with: "w1 ... d" */
  {
    _CW->nw = 1;
    _CW->d[0] = S;
    _CW->N = i - 1;
    assert(_CW->N <= POLY_Dmax + 1); /* Increase POLY_Dmax */
    for (j = 0; j < _CW->N; j++) {
      _CW->W[0][j] = IN[j];
      assert(IN[j] > 0);
    }
    goto MAP;
  }
  _CW->d[0] = IN[0]; /* ASSIGN (COMBINED) WEIGHTS */
  S = IN[0] * index;
  for (j = 1; j < i; j++) {
    if (S < IN[j]) {
      if (S) {
        puts("Error in INPUT: degree vs. weights!");
        exit(0);
      } else {
        _CW->d[++(_CW->nw)] = IN[j];
        S = IN[j] * index;
        if (1 == _CW->nw) {
          if (i % (1 + _CW->N)) {
            puts("INPUT error: numbers?");
            exit(0);
          }
        }
        if (j % (1 + _CW->N)) {
          puts("INPUT error: degrees?");
          exit(0);
        }
      }
    } else {
      _CW->W[_CW->nw][j - 1 - (1 + _CW->N) * _CW->nw] = IN[j];
      S -= IN[j];
      if (0 == _CW->nw)
        if (++(_CW->N) > AMBI_Dmax) {
          puts("Increase AMBI_Dmax!");
          exit(0);
        }
    }
  }
  ++(_CW->nw);
  checkDimension(_CW->N - _CW->nw, codim, index);

MAP:
  for (i = 0; i < _CW->nw; i++) /* check consistency of CWS */
  {
    Long sum = _CW->d[i] * index, *w = _CW->W[i];
    for (j = 0; j < _CW->N; j++) {
      assert(w[j] >= 0);
      sum -= w[j];
    }
    if ((sum) && (index == 1)) {
      printf("Use option -l for (single) WeightSystems with ");
      printf("d!=\\sum(w)\n(only Read_Weight makes the correct ");
      puts("PolyPointList in that case)");
      exit(0);
    }
  }
  _CW->nz = 0;
  if (!Read_CWS_Zinfo(inFILE, _CW)) /* read Z to EOL */
  {
    if (!InputOK)
      puts("-h gives you help\n");
    return 0;
  }
  Make_CWS_Points(_CW, _P);
  if (FilterFlag)
    inFILE = NULL;
  return 1;
}

int Read_CWS_PP(CWS *_CW, PolyPointList *_P) {
  return ReadCwsPp(_CW, _P, 1, 1);
}

int Read_PP(PolyPointList *_P) {
  int i, j, FilterFlag = (inFILE == NULL);
  int IN[AMBI_Dmax * (AMBI_Dmax + 1)];
  static int InputOK;
  /* _CW->nw=_CW->N=_CW->nz=0; */

  if (FilterFlag)
    inFILE = stdin;
  else if (inFILE == stdin) {
    printf("`#lines #columns' (= `PolyDim #Points' or `#Points PolyDim'):\n");
  };
  for (i = 0; i < AMBI_Dmax * (AMBI_Dmax + 1); i++) {
    char c;
    while (' ' == (c = fgetc(inFILE)))
      ;
    ungetc(c, inFILE); /* read blanks */
    if (IsNextDigit())
      fscanf(inFILE, "%d", &IN[i]);
    else
      break;
  }
  if (i == 0) {
    if (!InputOK) {
      puts("-h gives you help\n");
      exit(0);
    } else
      return 0;
  }
  InputOK++;
  if (i == 1) {
    puts("Error in INPUT: need at least 2 numbers!");
    exit(0);
  }
  if (i == 2) /* READ PolyPointList */
  {
    int tr = 0;
    while ('\n' != fgetc(inFILE))
      ; /* read to end of line */
    if (IN[0] == IN[1]) {
      puts("The number of points must be larger than the dimension!");
      exit(0);
    }
    if (IN[0] > IN[1]) {
      tr = IN[0];
      IN[0] = IN[1];
      IN[1] = tr;
    }
    tr = !tr;
    if (IN[0] > POLY_Dmax) {
      puts("increase POLY_Dmax!");
      exit(0);
    }
    if (IN[1] > POINT_Nmax) {
      puts("increase POINT_Nmax!");
      exit(0);
    }
    if ((inFILE == stdin) && !FilterFlag)
      printf("Type the %d coordinates as %s=%d lines with %s=%d columns:\n",
             IN[0] * IN[1], tr ? "dim" : "#pts", tr ? IN[0] : IN[1],
             tr ? "#pts" : "dim", tr ? IN[1] : IN[0]);
    _P->n = IN[0];
    _P->np = IN[1];
    /* allow all numbers in one string (distributed over lines) or
     * matrix blocks with trailing comments	*/
    if (tr)
      for (i = 0; i < IN[0]; i++)
        for (j = 0; j < IN[1]; j++) {
          int X;
          fscanf(inFILE, "%d", &X);
          _P->x[j][i] = X;
        }
    else
      for (i = 0; i < IN[1]; i++)
        for (j = 0; j < IN[0]; j++) {
          int X;
          fscanf(inFILE, "%d", &X);
          _P->x[i][j] = X;
        }
    /* Finish_Poly_Points(_P); */
    while (fgetc(inFILE) - '\n')
      if (feof(inFILE))
        return 0; /* read to EOL */
    if (FilterFlag)
      inFILE = NULL;
    return 1;
  }
  if (i > 2) {
    puts("Error: expected input format is matrix of polytope points!");
    exit(0);
  }
  if (FilterFlag)
    inFILE = NULL;
  return 1;
}

int Read_CWS(CWS *_CW, PolyPointList *_P) {
  int i, j, FilterFlag = (inFILE == NULL);
  int IN[AMBI_Dmax * (AMBI_Dmax + 1)], S;
  static int InputOK;
  _CW->nw = _CW->N = _CW->nz = 0;
  _CW->index = 1;

  if (FilterFlag)
    inFILE = stdin;
  else if (inFILE == stdin) {
    printf("Degrees and weights  `d1 w11 w12 ... d2 w21 w22 ...':\n");
  };
  for (i = 0; i < AMBI_Dmax * (AMBI_Dmax + 1); i++) {
    char c;
    while (' ' == (c = fgetc(inFILE)))
      ;
    ungetc(c, inFILE); /* read blanks */
    if (IsNextDigit())
      fscanf(inFILE, "%d", &IN[i]);
    else
      break;
  }
  if (i == 0) {
    if (!InputOK) {
      puts("-h gives you help\n");
      exit(0);
    } else
      return 0;
  }
  InputOK++;
  if (i == 1) {
    puts("Error in INPUT: need at least 2 numbers!");
    exit(0);
  }
  if (i == 2) {
    puts("Error: expected input format is CWS!");
    exit(0);
  }
  assert(i != 3);
  S = IN[i - 1];
  for (j = 0; j < i - 1; j++)
    if (S < IN[j])
      break;
  if (j == i - 1) /* Single Weights with: "w1 ... d" */
  {
    _CW->nw = 1;
    _CW->d[0] = S;
    _CW->N = i - 1;
    assert(_CW->N <= POLY_Dmax + 1); /* Increase POLY_Dmax */
    for (j = 0; j < _CW->N; j++) {
      _CW->W[0][j] = IN[j];
      assert(IN[j] > 0);
    }
    goto MAP;
  }
  S = _CW->d[_CW->nw] = IN[0]; /* ASSIGN WEIGHTS */
  for (j = 1; j < i; j++) {
    if (S < IN[j]) {
      if (S) {
        puts("Error in INPUT: degree vs. weights!");
        exit(0);
      } else {
        S = _CW->d[++(_CW->nw)] = IN[j];
        if (1 == _CW->nw) {
          if (i % (1 + _CW->N)) {
            puts("INPUT error: numbers?");
            exit(0);
          }
        }
        if (j % (1 + _CW->N)) {
          puts("INPUT error: degrees?");
          exit(0);
        }
      }
    } else {
      _CW->W[_CW->nw][j - 1 - (1 + _CW->N) * _CW->nw] = IN[j];
      S -= IN[j];
      if (0 == _CW->nw)
        if (++(_CW->N) > AMBI_Dmax) {
          puts("Increase AMBI_Dmax!");
          exit(0);
        }
    }
  }
  ++(_CW->nw);
  if (_CW->N - _CW->nw > POLY_Dmax) {
    printf("Please increase POLY_Dmax to at least %d\n", _CW->N - _CW->nw);
    exit(0);
  } /* increase POLY_Dmax */

MAP:
  for (i = 0; i < _CW->nw; i++) /* check consistency of CWS */
  {
    Long sum = _CW->d[i], *w = _CW->W[i];
    for (j = 0; j < _CW->N; j++) {
      assert(w[j] >= 0);
      sum -= w[j];
    }
    if (sum) { /*printf("Use poly.x -w for (single) WeightSystems with ");
                 printf("d!=\\sum(w)\n(only Read_Weight makes the correct ");
                 puts("PolyPointList in that case)"); exit(0);*/
      printf("cannot handle (single) WeightSystems with ");
      printf("d!=\\sum(w)\n");
      exit(0);
    }
  }
  _CW->nz = 0;
  if (!Read_CWS_Zinfo(inFILE, _CW)) /* read Z to EOL */
  {
    if (!InputOK)
      puts("-h gives you help\n");
    return 0;
  }
  Make_CWS_Points(_CW, _P); /* now make POLY */
  if (FilterFlag)
    inFILE = NULL;
  return 1;
}

void Print_CWH(CWS *_W, BaHo *_BH) {
  int i, j;
  for (i = 0; i < _W->nw; i++) {
    fprintf(outFILE, "%d ", (int)_W->d[i]);
    for (j = 0; j < _W->N; j++)
      fprintf(outFILE, "%d ", (int)_W->W[i][j]);
    if (i + 1 < _W->nw)
      fprintf(outFILE, " ");
  }
  Print_CWS_Zinfo(_W);
  if (_BH->np) {
    fprintf(outFILE, "M:%d %d N:%d %d", _BH->mp, _BH->mv, _BH->np, _BH->nv);
    if (_BH->n == 3)
      fprintf(outFILE, " Pic:%d Cor:%d", _BH->pic, _BH->cor);
    if (_BH->n > 3) {
      fprintf(outFILE, " H:%d", _BH->h1[1]);
      for (i = 2; i < _BH->n - 1; i++)
        fprintf(outFILE, ",%d", _BH->h1[i]);
      if (_BH->n == 4)
        fprintf(outFILE, " [%d]", 2 * (_BH->h1[1] - _BH->h1[2]));
      if (_BH->n == 5)
        fprintf(outFILE, " [%d]",
                48 + 6 * (_BH->h1[1] - _BH->h1[2] + _BH->h1[3]));
      if (_BH->n == 6)
        fprintf(outFILE, " [%d]",
                24 * (_BH->h1[1] - _BH->h1[2] + _BH->h1[3] - _BH->h1[4]));
    }
  } else if (_BH->mp)
    fprintf(outFILE, "M:%d %d F:%d", _BH->mp, _BH->mv, _BH->nv);
  else
    fprintf(outFILE, "V:%d F:%d", _BH->mv, _BH->nv);
  fprintf(outFILE, "\n");
}

/*  ==========  	      END of I/O functions		==========  */

/*  ==========	   Solve W-Eq. -> triangular LatticeBasis
 *
 *   B[0]:= (-n1, n0, 0, ...) / GCD(n0,n1);
 *   B[i]:= (0,...,0,g/G,0,...)- (ni/G) * ExtGCD.K(n0,...,n(i-1),0,...);
 *   	    with g=GCD(n0,...,n[i-1]); G=GCD(g,ni);
 *
 *   CWS by iteration: NextWeight=W[]*B; NewB=Basis(NextW); B[i+1]=B[i]*NewB;
 *									    */

void PrintBasis(CWLatticeBasis *_B) {
  int i, j;
  puts("Basis:");
  for (i = 0; i < _B->n; i++) {
    for (j = 0; j < _B->N; j++)
      fprintf(outFILE, "%6d ", (int)_B->x[i][j]);
    puts("");
  }
  puts("End of Basis  - -");
}

void Orig_Solve_Next_WEq(Long *NW, CWLatticeBasis *_B) {
  int i, j, P = 0, p[AMBI_Dmax];
  Long W[AMBI_Dmax], G;
  _B->n = _B->N - 1;
  for (i = 0; i < _B->N; i++) {
    for (j = 0; j < _B->n; j++)
      _B->x[j][i] = 0; /* init B.x=0 */
    if (NW[i]) {
      p[P] = i;
      W[P++] = NW[i];
    } /* non-zero weights */
  }
  if (P < 2)
    puts("need two non-zero weights in  >>Solve_Next_WEq<<");
  for (i = 0; i < p[0]; i++)
    _B->x[i][i] = 1;
  while ((++i) < p[1])
    _B->x[i - 1][i] = 1;
  G = Fgcd(W[0], W[1]);
  if (W[0] / G < 0)
    G = -G;
  _B->x[i - 1][p[0]] = -W[1] / G;
  _B->x[i - 1][p[1]] = W[0] / G;
  j = 2;
  while (++i < _B->N) {
    if (NW[i]) {
      int k;
      Long *X = _B->x[i - 1], K[AMBI_Dmax], g = REgcd(W, &j, K);
      G = Fgcd(g, NW[i]);
      if (g / G < 0)
        G = -G;
      X[i] = g / G;
      g = W[j] / G;
      for (k = 0; k < j; k++)
        X[p[k]] = -K[k] * g;
      j++;
    } else
      _B->x[i - 1][i] = 1;
  }
}
void Solve_Next_WEq(Long *NW, CWLatticeBasis *_B) {
  Long W[AMBI_Dmax], *X[AMBI_Dmax], GLZ[AMBI_Dmax][AMBI_Dmax];
  int i, j, P = 0, p[AMBI_Dmax];
  _B->n = _B->N - 1;
#ifdef TEST_Wbase
  Orig_Solve_Next_WEq(NW, _B);
  PrintBasis(_B);
#endif
  for (i = 0; i < _B->N; i++) {
    for (j = 0; j < _B->n; j++)
      _B->x[j][i] = 0; /* init B.x=0 */
    if (NW[i]) {
      p[P] = i;
      X[P] = GLZ[P];
      W[P++] = NW[i];
    } /* non-zero weights */
  }
  if (P > 1)
    W_to_GLZ(W, &P, X); /* P>1, compute GLZ */
  else { /* printf("P=%d W[0]=%d for W_to_GLZ\n",P,W[0]);exit(0);*/
    assert(P);
    for (i = 0; i < p[0]; i++)
      _B->x[i][i] = 1;
    while ((++i) < _B->N)
      _B->x[i - 1][i] = 1;
    return;
  }
  for (i = 1; i < P; i++)
    if (X[i][i] < 0)
      for (j = 0; j <= i; j++)
        X[i][j] *= -1;
  for (i = 0; i < p[0]; i++)
    _B->x[i][i] = 1;
  while ((++i) < p[1])
    _B->x[i - 1][i] = 1;
  _B->x[i - 1][p[0]] = X[1][0];
  _B->x[i - 1][p[1]] = X[1][1];
  j = 2;
  while (++i < _B->N) {
    if (NW[i]) {
      int k;
      Long *B = _B->x[i - 1];
      for (k = 0; k <= j; k++)
        B[p[k]] = X[j][k];
      j++;
    } else
      _B->x[i - 1][i] = 1;
  }
#ifdef TEST_Wbase
  /* for(i=0;i<_B->N;i++)printf("%d ",NW[i]);puts("=NW");for(i=0;i<P;i++)
  {for(j=0;j<P;j++)printf("%4d ",GLZ[i][j]);puts("=GL");} */
  printf("New version: ");
  PrintBasis(_B);
#endif
#ifdef USE_Old_Wbase
  Orig_Solve_Next_WEq(NW, _B);
#endif
}
void Make_CWS_Basis(CWS *_C, CWLatticeBasis *_B) {
  int i, j, k, l;
  Long W[AMBI_Dmax];
  CWLatticeBasis NB, AuxB;
  AuxB.N = _B->N = _C->N;
  Solve_Next_WEq(_C->W[0], _B);
  for (i = 1; i < _C->nw; i++) {
    for (j = 0; j < _B->n; j++) {
      W[j] = 0;
      for (k = 0; k < _B->N; k++)
        W[j] += _C->W[i][k] * _B->x[j][k];
    }
    NB.N = _B->n;
    Solve_Next_WEq(W, &NB);
    AuxB.n = NB.n;
    for (j = 0; j < AuxB.N; j++)
      for (k = 0; k < AuxB.n; k++) {
        AuxB.x[k][j] = 0;
        for (l = 0; l < NB.N; l++)
          AuxB.x[k][j] += NB.x[k][l] * _B->x[l][j];
      }
    *_B = AuxB;
  }
  /* init coordinate hyperplanes */
  _C->B.ne = _B->N; /* == transposed of Basis */
  for (i = 0; i < _B->N; i++) {
    for (j = 0; j < _B->n; j++)
      _C->B.e[i].a[j] = _B->x[j][i];
    _C->B.e[i].c = 1;
  }
}

void Poly_To_Ambi(CWLatticeBasis *_B, Long *x, Long *X) {
  int i, j;
  for (i = 0; i < _B->N; i++) {
    X[i] = 1;
    for (j = 0; j < _B->n; j++)
      X[i] += _B->x[j][i] * x[j];
  }
}
/*   B.x[i][A] is lower triangular: zero for i<p and Amin[p] <= A <Amin[p+1]
 *   X^A=x^p B_p^A+1, hence  for A \in [ Amin[p],Amin[p+1] ) and all k:
 *   x^iB_i^A \in [0,d_k/w_k^A] - 1 - \sum_{l<i} x^l B_l^A
 *									     */

void QuotZ_2_SublatG(Long Z[][VERT_Nmax], int *zm, Long *M, int *d,
                     Long G[][POLY_Dmax]);    /* normalize and diagonalize Z */
void CWS_2_SublatZ(CWS *C, CWLatticeBasis *B, /* in */
                   int *m, Long *M, Long G[POLY_Dmax][POLY_Dmax]) /* out */
{
  int i, j, k, d = C->N - C->nw;
  Long Z[POLY_Dmax][VERT_Nmax];
  *m = C->nz;
  for (i = 0; i < C->nz; i++) {
    M[i] = C->m[i];
    for (j = 0; j < d; j++) {
      Z[i][j] = 0;
      for (k = 0; k < C->N; k++)
        Z[i][j] += C->z[i][k] * B->x[j][k];
    }
  } /* PrintBasis(B); */
  QuotZ_2_SublatG(Z, m, M, &d, G);
}
void Reduce_PPL_2_Sublat(PolyPointList *P, int *nm, Long *M,
                         Long G[][POLY_Dmax]) {
  int i, j, n = 0, N;
  for (N = 0; N < P->np; N++) {
    Long X[POLY_Dmax];
    for (i = 0; i < P->n; i++) {
      X[i] = 0;
      for (j = 0; j < P->n; j++)
        X[i] += G[i][j] * P->x[N][j];
    }
    for (i = 0; i < *nm; i++)
      if (X[i] % M[i])
        break;
    if (i < *nm)
      continue;
    for (i = 0; i < *nm; i++)
      P->x[n][i] = X[i] / M[i];
    for (i = *nm; i < P->n; i++)
      P->x[n][i] = X[i];
    n++;
  }
  P->np = n; /* Print_PPL(P,"");printf("nm=%d\n",*nm); */
}
Long PD_Floor(Long N, Long D) /*  assuming PosDenom  D>0:  F <= N/D < F+1  */
{
  Long F = N / D;
  return (F * D > N) ? F - 1 : F;
}

void Old_Make_CWS_Points(CWS *Cin, PolyPointList *_P) {
  int i, j, Amin[POLY_Dmax + 1];
  Long *x = _P->x[_P->np = 0], xmin[POLY_Dmax], xmax[POLY_Dmax],
       Xmax[AMBI_Dmax], xaux[POLY_Dmax], L, R;
  CWS *_C = Cin;
  CWLatticeBasis B;
  Long G[POLY_Dmax][POLY_Dmax], M[POLY_Dmax];
  int m = Cin->nz;
#ifndef NO_COORD_IMPROVEMENT /* ==== Perm Coord Improvement ==== */
  int pi[AMBI_Dmax];
  CWS Caux;
  _C = &Caux;
  CWS_to_PermCWS(Cin, _C, pi);
#endif                       /* = End of Perm Coord Improvement = */
  Make_CWS_Basis(_C, &B);    /* make `triangular' Basis */
#ifndef NO_COORD_IMPROVEMENT /* ==== Perm Coord Improvement ==== */
  assert(_C->N == _C->B.ne);
  for (i = 0; i < Cin->N; i++)
    Cin->B.e[i] = _C->B.e[pi[i]];
  Cin->B.ne = _C->N;
#endif /* = End of Perm Coord Improvement = */
  i = _P->n = B.n;
  Amin[0] = 0;
  Amin[B.n] = j = B.N; /* inversion structure */
  while (--i) {
    while (!B.x[i - 1][--j])
      ;
    Amin[i] = ++j;
  }
  for (i = 0; i < B.N; i++) /* compute Xmax */
  {
    Xmax[i] = 0;
    for (j = 0; j < _C->nw; j++)
      if (_C->W[j][i]) {
        L = _C->d[j] / _C->W[j][i];
        if (Xmax[i]) {
          if (L < Xmax[i])
            Xmax[i] = L;
        } else
          Xmax[i] = L;
      }
  }
  j = B.n - 1;
  i = Amin[j + 1] - 1;
  R = B.x[j][i]; /* compute xmin[j] and xmax[j] */
  xmin[j] = -PD_Floor(1, R);
  xmax[j] = PD_Floor(Xmax[i] - 1, R); /* since R > 0 */
  /** /printf("R=%2d:  %2d <= x[%d=&%d] <= %2d\n",R,xmin[j],j,i,xmax[j]);/ **/
  while ((i--) > Amin[j]) /* if(R=B.x[B.n-1][i]):  R!=0  => new limits */
  {
    Long Low = -1, Upp = Low + Xmax[i];
    R = B.x[B.n - 1][i];
    if (R > 0) {
      if (xmax[j] > (L = PD_Floor(Upp, R)))
        xmax[j] = L;
      if (xmin[j] < (L = -PD_Floor(-Low, R)))
        xmin[j] = L;
    } else {
      if (xmax[j] > (L = PD_Floor(-Low, -R)))
        xmax[j] = L;
      if (xmin[j] < (L = -PD_Floor(Upp, -R)))
        xmin[j] = L;
    }
    /** /printf("R=%2d:  %2d <= x[%d=&%d] <= %2d\n",R,xmin[j],j,i,xmax[j]);/ **/
  } /* this completes the limits for x[B.n-1] */
  x[j] = xmin[j];
  while (j < B.n) {
    int k;
    if (x[j] > xmax[j]) {
      if (B.n == (++j))
        break;
      else
        x[j]++;
    } else /* compute limits[j-1] and initialize x[j-1] */
    {
      Long Low = -1, Upp = Xmax[i = Amin[j--] - 1];
      int RangeFlag = 0;
      for (k = j + 1; k < B.n; k++)
        Low -= x[k] * B.x[k][i]; /* compute offset */
      Upp += Low;
      R = B.x[j][i];
      xmin[j] = -PD_Floor(-Low, R);
      xmax[j] = PD_Floor(Upp, R);
      /** /printf("R=%2d:  %2d <= x[%d=&%d] <= %2d\n",R,xmin[j],j,i,xmax[j]);/
       * **/
      while ((i--) > Amin[j])
        if ((R = B.x[j][i])) /* R!=0  => new limits */
        {
          Low = -1;
          Upp = Xmax[i];
          for (k = j + 1; k < B.n; k++)
            Low -= x[k] * B.x[k][i];
          Upp += Low;
          R = B.x[j][i];
          if (R > 0) {
            if (xmax[j] > (L = PD_Floor(Upp, R)))
              xmax[j] = L;
            if (xmin[j] < (L = -PD_Floor(-Low, R)))
              xmin[j] = L;
          } else {
            if (xmax[j] > (L = PD_Floor(-Low, -R)))
              xmax[j] = L;
            if (xmin[j] < (L = -PD_Floor(Upp, -R)))
              xmin[j] = L;
          }
          /** /printf("R=%2d:  %2d <= x[%d=&%d] <=
           * %2d\n",R,xmin[j],j,i,xmax[j]);/ **/
        } /* completes limits for x[] except */
        else /*   when R=0 and X out of Range:  */
        {
          Long X = 1;
          for (k = j + 1; k < B.n; k++)
            X += x[k] * B.x[k][i];
          if ((X < 0) || (X > Xmax[i]))
            RangeFlag = 1;
        }
      if (RangeFlag)
        ++x[++j];
      else
        x[j] = xmin[j];
      if (j == 0) {
        while (x[0] <= xmax[0]) {
          Long *y;
          if ((++_P->np) < POINT_Nmax) {
            y = (_P->x[_P->np]);
            for (k = 0; k < B.n; k++)
              y[k] = x[k];
          } else if (_P->np == POINT_Nmax) {
            y = xaux;
            for (k = 0; k < B.n; k++)
              y[k] = x[k];
          } else {
            puts("Increase POINT_Nmax");
            exit(0);
          }
          x = y;
          ++x[0];
        }
        x[j = 1]++;
      }
    }
  }
  if (m)
    CWS_2_SublatZ(_C, &B, &m, M, G);
  if (m)
    Reduce_PPL_2_Sublat(_P, &m, M, G);
}

int Compute_X0(int N, CWS *_C, Long *X0) {
  /* finds a first X = X0 such that sum w_i X_i = d for every weight w in _C,
     this becomes the origin after the change of coordinates from X to x */
  int j;
  Long Xmax = 0;
  if (!N) {
    for (j = 0; j < _C->nw; j++)
      if (_C->W[j][0]) {
        if (_C->d[j] % _C->W[j][N])
          return 0;
        if (Xmax) {
          if (Xmax != _C->d[j] / _C->W[j][N])
            return 0;
        } else
          Xmax = _C->d[j] / _C->W[j][N];
      } else if (_C->d[j])
        return 0;
    X0[0] = Xmax;
    return 1;
  }
  for (j = 0; j < _C->nw; j++)
    if (_C->W[j][N] != 0) {
      Long L = _C->d[j] / _C->W[j][N];
      if (Xmax) {
        if (L < Xmax)
          Xmax = L;
      } else
        Xmax = L;
    }
  for (X0[N] = 0; X0[N] <= Xmax; X0[N]++) {
    if (Compute_X0(N - 1, _C, X0)) {
      for (j = 0; j < _C->nw; j++)
        _C->d[j] += X0[N] * _C->W[j][N]; /* reset Cin*/
      return 1;
    }
    for (j = 0; j < _C->nw; j++)
      _C->d[j] -= _C->W[j][N];
  }
  for (j = 0; j < _C->nw; j++)
    _C->d[j] += (Xmax + 1) * _C->W[j][N]; /* reset Cin */
  return 0;
}

/* ---------- Helper: compute bounds for polytope coordinate x[j] ----------
 * Bj       = B.x[j], the j-th basis row (triangular: first pivot > 0)
 * offset   = accumulated contributions from higher coordinates
 *            (offset[A] = sum_{k>j} x[k]*B.x[k][A])
 * X0, Xmax = per-ambient-coordinate origin and upper-bound constants
 * Alo, Ahi = ambient coordinate range for this level [Amin[j], Amin[j+1])
 * Returns 1 if feasible, 0 if infeasible (R=0 range violation). */
static inline int CLB(const Long *Bj, const Long *offset,
                      const Long *X0, const Long *Xmax,
                      int Alo, int Ahi, Long *lo, Long *hi) {
  int A = Ahi - 1;
  Long R = Bj[A]; /* first pivot always positive (triangular basis) */
  Long Low = -X0[A] - offset[A];
  Long Upp = Low + Xmax[A];
  Long L;
  *lo = -PD_Floor(-Low, R);
  *hi = PD_Floor(Upp, R);
  while (--A >= Alo) {
    if ((R = Bj[A])) {
      Low = -X0[A] - offset[A];
      Upp = Low + Xmax[A];
      if (R > 0) {
        if (*hi > (L = PD_Floor(Upp, R))) *hi = L;
        if (*lo < (L = -PD_Floor(-Low, R))) *lo = L;
      } else {
        if (*hi > (L = PD_Floor(-Low, -R))) *hi = L;
        if (*lo < (L = -PD_Floor(Upp, -R))) *lo = L;
      }
    } else {
      Long X = X0[A] + offset[A];
      if (X < 0 || X > Xmax[A]) return 0;
    }
  }
  return 1;
}

#ifdef LLLFP_WALK
/* ============================================================================
 *  LLL-reduced basis + Fincke-Pohst lattice-point walk (dim-5 fast path).
 *
 *  Replaces the triangular 5-nested-loop enumeration with a near-orthogonal
 *  basis (LLL-reduced in the box-scaled metric <u,v> = sum_A u_A v_A / r_A^2,
 *  r_A = Xmax_A/2) enumerated by integer box-constraint propagation over the
 *  circumscribed-ellipsoid root box.  It enumerates the IDENTICAL set of
 *  lattice points; the reduced-basis coefficients xp are mapped back to the
 *  original triangular-basis coefficients via the unimodular U (b'_i = U_ij b_j),
 *  so _P->x and all downstream code (sublattice reduction, IP_Check) are
 *  byte-for-byte unaffected.  Correctness: U integer with |det U| = 1 => same
 *  lattice; ellipsoid root box is a true superset of the feasible region; the
 *  per-leaf box test is exact integer.  See repo LLL_FP_WALK.md.
 *
 *  Compiled only with -DLLLFP_WALK (stock build is unchanged).  Mode via the
 *  PALP_WALK env var: unset/"fp" = use this walk, "tri" = keep triangular,
 *  "check" = run both and assert set-equality (validation; output stays the
 *  trusted triangular result).
 *
 *  No libm dependency: fabs/floor/ceil/round are inlined below and sqrt uses
 *  __builtin_sqrt (baseline SSE2 sqrtsd; build with -fno-math-errno).
 * ========================================================================== */

#define LF_N 5

static inline double lf_fabs(double x) { return x < 0 ? -x : x; }
static inline long   lf_floorL(double v) { long f = (long)v; if ((double)f > v) f--; return f; }
static inline long   lf_ceilL (double v) { long f = (long)v; if ((double)f < v) f++; return f; }
static inline long long lf_lround(double v) { return (long long)(v >= 0 ? v + 0.5 : v - 0.5); }

static int lf_mode(void) {        /* 0=tri, 1=fp(default), 2=check; cached */
  static int m = -1;
  if (m < 0) { const char *s = getenv("PALP_WALK");
    if (!s || !*s) m = 1;
    else if (!strcmp(s, "tri")) m = 0;
    else if (!strcmp(s, "check")) m = 2;
    else m = 1; }
  return m;
}

/* The circumscribed-ellipsoid root box is a *superset*; for some bases it is
 * loose enough that the walk would visit a huge empty subtree.  We bound the
 * internal-node count per candidate and, on exceeding it, fall back to the
 * proven triangular walk for that candidate (correct and finite).  Generous
 * vs the realistic per-candidate node count (~1e2..1e6), tight enough to bound
 * a runaway to well under a second.  Tunable via PALP_LF_NODECAP. */
#define LF_NODE_CAP_DEFAULT 8000000LL
static long long lf_node_cap(void) {
  static long long c = -1;
  if (c < 0) { const char *s = getenv("PALP_LF_NODECAP"); c = (s && *s) ? atoll(s) : LF_NODE_CAP_DEFAULT; if (c < 1) c = LF_NODE_CAP_DEFAULT; }
  return c;
}
/* Heavy-tail gate: the LLL + ellipsoid setup costs ~1e4 cycles/candidate, so fp
 * only pays off on candidates whose triangular walk would cost much more than
 * that.  The bulk of candidates are light (small box, np~9), where the overhead
 * is pure loss.  Gate on a cheap pre-walk proxy for box size -- sum of the
 * bit-lengths of Xmax (~ log2 of the box volume).  PALP_LF_LOGVOL_MIN sets the
 * threshold; 0 (default) = gate off = always fp (legacy behaviour). */
static long long lf_logvol_min(void) {
  static long long t = -2;
  if (t == -2) { const char *s = getenv("PALP_LF_LOGVOL_MIN"); t = (s && *s) ? atoll(s) : 0; }
  return t;
}
static int lf_heavy(const Long *Xmax, int N) {
  long long thr = lf_logvol_min();
  if (thr <= 0) return 1;
  long long lv = 0;
  for (int A = 0; A < N; A++) { Long x = Xmax[A]; while (x > 0) { lv++; x >>= 1; } }
  return lv >= thr;
}
/* fp-walk accounting (candidates handled vs fallbacks); reported at exit. */
static long long lf_fp_tot = 0, lf_fp_abort = 0, lf_fp_light = 0;
static void lf_fp_report(void) {
  static int done = 0; if (done) return; done = 1;
  long long seen = lf_fp_tot + lf_fp_light;
  if (seen)
    fprintf(stderr, "[LFFP] candidates=%lld  fp-walk=%lld  light(triangular-gate)=%lld"
            "  fp-node-fallback=%lld\n", seen, lf_fp_tot, lf_fp_light, lf_fp_abort);
}

static inline long long lf_fdiv(long long a, long long b) {   /* floor(a/b) */
  long long q = a / b, r = a % b; if (r && ((a < 0) != (b < 0))) q--; return q; }
static inline long long lf_cdiv(long long a, long long b) {   /* ceil(a/b) */
  long long q = a / b, r = a % b; if (r && ((a < 0) == (b < 0))) q++; return q; }

/* Gram-Schmidt in the box-scaled metric (weights w[A] = 1/r_A^2). */
static void lf_gso(Long b[LF_N][AMBI_Dmax], int n, int N, const double *w,
                   double mu[LF_N][LF_N], double *B2) {
  double bs[LF_N][AMBI_Dmax];
  for (int i = 0; i < n; i++) {
    for (int A = 0; A < N; A++) bs[i][A] = (double)b[i][A];
    for (int j = 0; j < i; j++) {
      double dot = 0, nj = B2[j];
      for (int A = 0; A < N; A++) dot += (double)b[i][A] * bs[j][A] * w[A];
      mu[i][j] = (nj > 0) ? dot / nj : 0.0;
      for (int A = 0; A < N; A++) bs[i][A] -= mu[i][j] * bs[j][A];
    }
    double nn = 0; for (int A = 0; A < N; A++) nn += bs[i][A] * bs[i][A] * w[A];
    B2[i] = nn;
  }
}
/* exact determinant of a small integer matrix (fraction-free Bareiss) */
static long long lf_idet(long long M[LF_N][LF_N], int n) {
  long long prev = 1;
  for (int k = 0; k < n; k++) {
    if (M[k][k] == 0) { int s = -1;
      for (int i = k + 1; i < n; i++) if (M[i][k]) { s = i; break; }
      if (s < 0) return 0;
      for (int j = 0; j < n; j++) { long long t = M[k][j]; M[k][j] = M[s][j]; M[s][j] = t; }
      prev = -prev; }
    for (int i = k + 1; i < n; i++)
      for (int j = k + 1; j < n; j++)
        M[i][j] = (M[i][j] * M[k][k] - M[i][k] * M[k][j]) / prev;
    prev = M[k][k];
  }
  return M[n - 1][n - 1];
}
/* LLL-reduce b (rows) in the scaled metric; fill U (b'_i = sum_j U_ij b_j_orig);
 * return det(U) (mathematically always +-1; checked defensively by caller). */
static long long lf_lll_reduce(Long b[LF_N][AMBI_Dmax], int n, int N,
                               const double *w, double delta, long long U[LF_N][LF_N]) {
  for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) U[i][j] = (i == j);
  double mu[LF_N][LF_N], B2[LF_N];
  lf_gso(b, n, N, w, mu, B2);
  int k = 1, guard = 0;
  while (k < n && guard++ < 100000) {
    for (int l = k - 1; l >= 0; l--) {
      if (lf_fabs(mu[k][l]) > 0.5) {
        long long q = lf_lround(mu[k][l]);
        if (q) {
          for (int A = 0; A < N; A++) b[k][A] -= q * b[l][A];
          for (int j = 0; j < n; j++) U[k][j] -= q * U[l][j];
          /* size reduction leaves the GSO vectors b* (and B2) unchanged; only
           * the mu coefficients shift -- update them in O(n), no full re-GSO. */
          for (int j = 0; j < l; j++) mu[k][j] -= (double)q * mu[l][j];
          mu[k][l] -= (double)q;
        }
      }
    }
    if (B2[k] >= (delta - mu[k][k - 1] * mu[k][k - 1]) * B2[k - 1]) k++;
    else {
      for (int A = 0; A < N; A++) { Long t = b[k][A]; b[k][A] = b[k - 1][A]; b[k - 1][A] = t; }
      for (int j = 0; j < n; j++) { long long t = U[k][j]; U[k][j] = U[k - 1][j]; U[k - 1][j] = t; }
      lf_gso(b, n, N, w, mu, B2);
      k = (k - 1 > 1) ? k - 1 : 1;
    }
  }
  long long Uc[LF_N][LF_N];
  for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) Uc[i][j] = U[i][j];
  return lf_idet(Uc, n);
}
/* solve G x = rhs (SPD, n<=5) by Gauss-Jordan; also return inverse diagonal */
static int lf_spd_solve(double G[LF_N][LF_N], int n, double *rhs, double *x, double *invdiag) {
  double A[LF_N][2 * LF_N];
  for (int i = 0; i < n; i++) { for (int j = 0; j < n; j++) { A[i][j] = G[i][j]; A[i][n + j] = (i == j); } }
  for (int c = 0; c < n; c++) {
    int piv = c; double best = lf_fabs(A[c][c]);
    for (int r = c + 1; r < n; r++) if (lf_fabs(A[r][c]) > best) { best = lf_fabs(A[r][c]); piv = r; }
    if (best < 1e-12) return 0;
    if (piv != c) for (int j = 0; j < 2 * n; j++) { double t = A[c][j]; A[c][j] = A[piv][j]; A[piv][j] = t; }
    double d = A[c][c]; for (int j = 0; j < 2 * n; j++) A[c][j] /= d;
    for (int r = 0; r < n; r++) if (r != c) { double f = A[r][c]; for (int j = 0; j < 2 * n; j++) A[r][j] -= f * A[c][j]; }
  }
  for (int i = 0; i < n; i++) { invdiag[i] = A[i][n + i];
    double s = 0; for (int j = 0; j < n; j++) s += A[i][n + j] * rhs[j]; x[i] = s; }
  return 1;
}

typedef struct {
  int n, N;
  Long b[LF_N][AMBI_Dmax];        /* LLL-reduced basis b' (rows)            */
  long long U[LF_N][LF_N];        /* b'_i = sum_j U_ij b_orig_j             */
  const Long *X0, *Xmax;
  Long Lo[LF_N], Hi[LF_N];        /* root search box (superset)             */
  int  ord[LF_N];                 /* enumeration order, outer..inner        */
  Long xp[LF_N];                  /* current coeff per basis-row index      */
  PolyPointList *P;
  long long nodes, cap;           /* internal-node budget (runaway guard)   */
  int abort;                      /* set when cap exceeded -> caller falls back */
} LFWalk;

static inline void lf_store(LFWalk *g) {        /* map xp via U, store B-coords */
  PolyPointList *P = g->P;
  if (P->np >= POINT_Nmax) { puts("Increase POINT_Nmax"); exit(0); }
  Long *out = P->x[P->np++];
  for (int j = 0; j < g->n; j++) {
    long long s = 0;
    for (int i = 0; i < g->n; i++) s += g->xp[i] * g->U[i][j];
    out[j] = (Long)s;
  }
}
static void lf_rec(LFWalk *g, int depth, const Long *accum) {
  int n = g->n, N = g->N;
  if (depth == n) {                              /* leaf: exact integer box test */
    for (int A = 0; A < N; A++)
      if (accum[A] < 0 || accum[A] > g->Xmax[A]) return;
    lf_store(g);
    return;
  }
  if (++g->nodes > g->cap) { g->abort = 1; return; }   /* runaway guard */
  int w = g->ord[depth];
  Long lo = g->Lo[w], hi = g->Hi[w];
  for (int A = 0; A < N && lo <= hi; A++) {
    Long c = g->b[w][A];
    long long sLo = 0, sHi = 0;                  /* deeper free vars' range on A */
    for (int d = depth + 1; d < n; d++) {
      int j = g->ord[d]; Long bb = g->b[j][A];
      if (bb > 0) { sLo += (long long)bb * g->Lo[j]; sHi += (long long)bb * g->Hi[j]; }
      else        { sLo += (long long)bb * g->Hi[j]; sHi += (long long)bb * g->Lo[j]; }
    }
    long long base = accum[A];
    long long K1 = -base - sHi;                  /* need c*x_w >= K1 */
    long long K2 = (long long)g->Xmax[A] - base - sLo; /* and c*x_w <= K2 */
    if (c > 0) { long long t = lf_cdiv(K1, c); if (t > lo) lo = t;
                 t = lf_fdiv(K2, c); if (t < hi) hi = t; }
    else if (c < 0) { long long t = lf_fdiv(K1, c); if (t < hi) hi = t;
                      t = lf_cdiv(K2, c); if (t > lo) lo = t; }
    else { if (base + sHi < 0 || base + sLo > g->Xmax[A]) return; }  /* c==0: prune */
  }
  Long child[AMBI_Dmax];
  for (Long x = lo; x <= hi && !g->abort; x++) {
    g->xp[w] = x;
    for (int A = 0; A < N; A++) child[A] = accum[A] + x * g->b[w][A];
    lf_rec(g, depth + 1, child);
  }
}
/* The walk: LLL-reduce, build ellipsoid root box, enumerate, store into P.
 * Returns 1 on success, 0 if the node budget was exceeded (caller must reset
 * P->np and fall back to the triangular walk). */
static int lf_walk_dim5(const CWLatticeBasis *B, const Long *X0,
                        const Long *Xmax, PolyPointList *P) {
  LFWalk g;
  int n = B->n, N = B->N, i, j, A;
  g.n = n; g.N = N; g.X0 = X0; g.Xmax = Xmax; g.P = P;
  g.nodes = 0; g.cap = lf_node_cap(); g.abort = 0;
  for (i = 0; i < n; i++) for (A = 0; A < N; A++) g.b[i][A] = B->x[i][A];

  double w[AMBI_Dmax];
  for (A = 0; A < N; A++) { double rr = (Xmax[A] > 0) ? Xmax[A] / 2.0 : 0.5; w[A] = 1.0 / (rr * rr); }
  long long det = lf_lll_reduce(g.b, n, N, w, 0.99, g.U);
  if (det != 1 && det != -1) {       /* defensive: revert to triangular basis */
    for (i = 0; i < n; i++) { for (A = 0; A < N; A++) g.b[i][A] = B->x[i][A];
                              for (j = 0; j < n; j++) g.U[i][j] = (i == j); }
  }
  /* circumscribed ellipsoid of {0<=X_A<=Xmax_A}: sum_A ((X_A-cc_A)/r_A)^2 <= rho */
  double cc[AMBI_Dmax], r[AMBI_Dmax], t[AMBI_Dmax], rho = 0, tt = 0;
  for (A = 0; A < N; A++) { if (Xmax[A] > 0) { cc[A] = Xmax[A] / 2.0; r[A] = Xmax[A] / 2.0; rho += 1.0; }
                            else { cc[A] = 0.0; r[A] = 0.5; } }
  for (A = 0; A < N; A++) { t[A] = (cc[A] - X0[A]) / r[A]; tt += t[A] * t[A]; }
  double G[LF_N][LF_N], gv[LF_N];
  for (i = 0; i < n; i++) {
    gv[i] = 0; for (A = 0; A < N; A++) gv[i] += (g.b[i][A] / r[A]) * t[A];
    for (j = 0; j < n; j++) {
      double s = 0;
      for (A = 0; A < N; A++) s += (g.b[i][A] / r[A]) * (g.b[j][A] / r[A]);
      G[i][j] = s;
    }
  }
  double xhat[LF_N], invdiag[LF_N];
  if (!lf_spd_solve(G, n, gv, xhat, invdiag)) { for (j = 0; j < n; j++) { xhat[j] = 0; invdiag[j] = 0; } }
  double res = tt; for (i = 0; i < n; i++) res -= gv[i] * xhat[i];
  double rho2 = rho - res + 1e-6; if (rho2 < 0) rho2 = 0;
  for (j = 0; j < n; j++) { double hw = (invdiag[j] > 0) ? __builtin_sqrt(rho2 * invdiag[j]) : 0.0;
    g.Lo[j] = lf_floorL(xhat[j] - hw) - 2;       /* +/-2 guard vs FP error */
    g.Hi[j] = lf_ceilL (xhat[j] + hw) + 2; }
  /* enumeration order: longest GSO vector (scaled metric) outermost */
  { double mu[LF_N][LF_N], B2[LF_N]; Long tmp[LF_N][AMBI_Dmax];
    for (i = 0; i < n; i++) for (A = 0; A < N; A++) tmp[i][A] = g.b[i][A];
    lf_gso(tmp, n, N, w, mu, B2);
    int used[LF_N]; for (j = 0; j < n; j++) used[j] = 0;
    for (int s = 0; s < n; s++) { int best = -1; double bv = -1;
      for (j = 0; j < n; j++) if (!used[j] && B2[j] > bv) { bv = B2[j]; best = j; }
      used[best] = 1; g.ord[s] = best; } }
  Long accum0[AMBI_Dmax]; for (A = 0; A < N; A++) accum0[A] = X0[A];
  lf_rec(&g, 0, accum0);
  lf_fp_tot++;
  if (g.abort) { lf_fp_abort++; return 0; }
  return 1;
}

/* Dispatch: gate light candidates to the triangular walk; run fp on heavy ones.
 * Returns 1 if P was filled (caller skips the triangular path), 0 otherwise. */
static int lf_fp_enumerate(const CWLatticeBasis *B, const Long *X0,
                           const Long *Xmax, PolyPointList *P) {
  static int reg = 0; if (!reg) { atexit(lf_fp_report); reg = 1; }
  if (!lf_heavy(Xmax, B->N)) { lf_fp_light++; return 0; }  /* light -> triangular */
  return lf_walk_dim5(B, X0, Xmax, P);                     /* heavy: fp (0 if it fell back) */
}

/* ---- check mode: run the FP walk into a scratch list and compare the point
 *      SET (order-independent 128-bit multiset hash) against the triangular
 *      result already in Ptri; report mismatches.  Used for validation only. */
static long long lf_chk_tot = 0, lf_chk_ok = 0, lf_chk_bad = 0,
                 lf_chk_fb = 0, lf_chk_max = -1;
static void lf_chk_report(void) {
  static int done = 0; if (done) return; done = 1;
  fprintf(stderr, "[LFCHK] candidates=%lld  set-identical=%lld  mismatch=%lld"
          "  fp-fallback=%lld\n", lf_chk_tot, lf_chk_ok, lf_chk_bad, lf_chk_fb);
}
static void lf_set_hash(const PolyPointList *P, unsigned long long *h1, unsigned long long *h2) {
  unsigned long long a1 = 0, a2 = 0;
  for (int p = 0; p < P->np; p++) {
    unsigned long long x1 = 1469598103934665603ULL, x2 = 14695981039346656037ULL;
    for (int j = 0; j < P->n; j++) {
      unsigned long long v = (unsigned long long)(long long)P->x[p][j];
      x1 = (x1 ^ v) * 1099511628211ULL;
      x2 = (x2 ^ (v + 0x9e3779b97f4a7c15ULL)) * 1099511628211ULL;
    }
    a1 += (x1 | 1ULL); a2 += (x2 | 1ULL);
  }
  *h1 = a1; *h2 = a2;
}
static void lf_check_dim5(const CWLatticeBasis *B, const Long *X0,
                          const Long *Xmax, const PolyPointList *Ptri, long idx) {
  static PolyPointList *Pf = NULL;
  static int reg = 0;
  if (!reg) { atexit(lf_chk_report); reg = 1;
    const char *s = getenv("PALP_LFCHK_MAX"); lf_chk_max = (s && *s) ? atoll(s) : 0; }
  if (!Pf) { Pf = (PolyPointList *)malloc(sizeof(PolyPointList));
             if (!Pf) { fprintf(stderr, "[LFCHK] malloc failed\n"); return; } }
  Pf->n = Ptri->n; Pf->np = 0;
  int fp_ok = lf_walk_dim5(B, X0, Xmax, Pf);
  lf_chk_tot++;
  if (!fp_ok) { lf_chk_fb++; }       /* fp hit node budget -> triangular fallback */
  else {
    int ok = (Pf->np == Ptri->np);
    if (ok) { unsigned long long a1, a2, b1, b2;
      lf_set_hash(Ptri, &a1, &a2); lf_set_hash(Pf, &b1, &b2);
      ok = (a1 == b1 && a2 == b2); }
    if (ok) lf_chk_ok++;
    else { lf_chk_bad++;
      fprintf(stderr, "[LFCHK] MISMATCH idx=%ld N=%d tri_np=%d fp_np=%d\n",
              idx, B->N, Ptri->np, Pf->np); }
  }
  if ((lf_chk_tot % 500000) == 0)
    fprintf(stderr, "[LFCHK] progress: candidates=%lld mismatch=%lld\n",
            lf_chk_tot, lf_chk_bad);
  if (lf_chk_max > 0 && lf_chk_tot >= lf_chk_max) {   /* bounded clean exit */
    lf_chk_report(); fflush(NULL); exit(0);
  }
}
#endif /* LLLFP_WALK */

void Make_CWS_Points(CWS *Cin, PolyPointList *_P) {
  int i, j, Amin[POLY_Dmax + 1], m = Cin->nz;
  Long Xmax[AMBI_Dmax], X0[AMBI_Dmax];
  CWS *_C = Cin;
  CWLatticeBasis B;
  Long G[POLY_Dmax][POLY_Dmax], M[POLY_Dmax];

  _P->np = 0;

#ifndef NO_COORD_IMPROVEMENT /* ==== Perm Coord Improvement ==== */
  int pi[AMBI_Dmax];
  CWS Caux;
  _C = &Caux;
  CWS_to_PermCWS(Cin, _C, pi);
#endif                       /* = End of Perm Coord Improvement = */
  Make_CWS_Basis(_C, &B);    /* make `triangular' Basis */
#ifndef NO_COORD_IMPROVEMENT /* ==== Perm Coord Improvement ==== */
  assert(_C->N == _C->B.ne);
  for (i = 0; i < Cin->N; i++)
    Cin->B.e[i] = _C->B.e[pi[i]];
  Cin->B.ne = _C->N;
#endif /* = End of Perm Coord Improvement = */
  if (Cin->index == 1)
    for (i = 0; i < Cin->N; i++) X0[i] = 1;
  else if (!Compute_X0(Cin->N - 1, Cin, X0)) {
    _P->n = 0;
    puts("no X0!");
    return;
  }

  i = _P->n = B.n;
  if (B.n == 0) goto do_sublat;
  Amin[0] = 0;
  Amin[B.n] = j = B.N;
  while (--i) {
    while (!B.x[i - 1][--j])
      ;
    Amin[i] = ++j;
  }
  for (i = 0; i < B.N; i++) {
    Long L;
    Xmax[i] = 0;
    for (j = 0; j < _C->nw; j++)
      if (_C->W[j][i]) {
        L = _C->d[j] / _C->W[j][i];
        if (Xmax[i]) { if (L < Xmax[i]) Xmax[i] = L; }
        else Xmax[i] = L;
      }
  }

#if POLY_Dmax >= 5
  /* ====== Dimension-5 fast path ======
   * Five nested for-loops with incremental level-accumulator arrays.
   *  - Offset sum_{k>j} x[k]*B[k][A] is maintained incrementally via
   *    lev arrays, reducing per-step cost from O(n-j) multiply-adds to
   *    O(1) additions at the innermost levels.
   *  - Static loop structure enables better register allocation.
   *  - Batch point storage in the innermost sweep removes per-point
   *    pointer management and bounds checks.
   */
  if (B.n == 5) {
    const int A0 = 0, A1 = Amin[1], A2 = Amin[2],
              A3 = Amin[3], A4 = Amin[4], A5 = Amin[5];
    Long lev4[AMBI_Dmax], lev3[AMBI_Dmax],
         lev2[AMBI_Dmax], lev1[AMBI_Dmax];
    Long xmn4, xmx4, xmn3, xmx3, xmn2, xmx2, xmn1, xmx1, xmn0, xmx0;
    int A;

#ifdef LLLFP_WALK
    /* LLL + Fincke-Pohst replacement (see block above). fp: enumerate via the
     * reduced basis and skip the triangular loops entirely.  If the walk
     * exceeds its node budget it returns 0; reset and fall through to the
     * (always-correct, finite) triangular path for this candidate. */
    if (lf_mode() == 1) {
      if (lf_fp_enumerate(&B, X0, Xmax, _P)) goto do_sublat;
      _P->np = 0;   /* light candidate or fp fell back -> triangular walk below */
    }
#endif

    /* Outermost bounds for x4 (no offset) */
    { Long zero[AMBI_Dmax];
      for (A = 0; A < A5; A++) zero[A] = 0;
      if (!CLB(B.x[4], zero, X0, Xmax, A4, A5, &xmn4, &xmx4))
        goto do_sublat;
    }

    /* Pre-seed lev4 one step before xmn4 for incremental update */
    for (A = 0; A < A4; A++) lev4[A] = (xmn4 - 1) * B.x[4][A];

    for (Long x4 = xmn4; x4 <= xmx4; x4++) {
      for (A = 0; A < A4; A++) lev4[A] += B.x[4][A];
      if (!CLB(B.x[3], lev4, X0, Xmax, A3, A4, &xmn3, &xmx3))
        continue;

      for (A = 0; A < A3; A++)
        lev3[A] = lev4[A] + (xmn3 - 1) * B.x[3][A];

      for (Long x3 = xmn3; x3 <= xmx3; x3++) {
        for (A = 0; A < A3; A++) lev3[A] += B.x[3][A];
        if (!CLB(B.x[2], lev3, X0, Xmax, A2, A3, &xmn2, &xmx2))
          continue;

        for (A = 0; A < A2; A++)
          lev2[A] = lev3[A] + (xmn2 - 1) * B.x[2][A];

        for (Long x2 = xmn2; x2 <= xmx2; x2++) {
          for (A = 0; A < A2; A++) lev2[A] += B.x[2][A];
          if (!CLB(B.x[1], lev2, X0, Xmax, A1, A2, &xmn1, &xmx1))
            continue;

          for (A = A0; A < A1; A++)
            lev1[A] = lev2[A] + (xmn1 - 1) * B.x[1][A];

          for (Long x1 = xmn1; x1 <= xmx1; x1++) {
            for (A = A0; A < A1; A++) lev1[A] += B.x[1][A];
            if (!CLB(B.x[0], lev1, X0, Xmax, A0, A1, &xmn0, &xmx0))
              continue;

            /* ---- Batch store x0 sweep ---- */
            { int cnt = (int)(xmx0 - xmn0 + 1);
              if (cnt > 0) {
                if (_P->np + cnt > POINT_Nmax) {
                  puts("Increase POINT_Nmax"); exit(0);
                }
                for (int c = 0; c < cnt; c++) {
                  Long *p = _P->x[_P->np++];
                  p[0] = xmn0 + c;
                  p[1] = x1; p[2] = x2; p[3] = x3; p[4] = x4;
                }
              }
            }
          } /* x1 */
        } /* x2 */
      } /* x3 */
    } /* x4 */
#ifdef DUMP_BASIS
    /* Dump the triangular enumeration basis (rows) + X0 + Xmax + np + index per
     * candidate, for the offline LLL / Fincke-Pohst node-count prototype
     * (scripts/fp_enum.c).  Inert unless built with -DDUMP_BASIS; gated by the
     * DUMP_STRIDE env var so a sparse representative sample can be collected. */
    { static long dc = 0, ds = -1;
      if (ds < 0) { const char *s = getenv("DUMP_STRIDE"); ds = (s && *s) ? atol(s) : 1; if (ds < 1) ds = 1; }
      if ((dc++ % ds) == 0) {
        int jj, A;
        fprintf(stderr, "BASIS n=%d N=%d np=%d idx=%ld", B.n, B.N, _P->np, (long)Cin->index);
        for (jj = 0; jj < B.n; jj++) { fprintf(stderr, " |"); for (A = 0; A < B.N; A++) fprintf(stderr, " %ld", (long)B.x[jj][A]); }
        fprintf(stderr, " |0"); for (A = 0; A < B.N; A++) fprintf(stderr, " %ld", (long)X0[A]);
        fprintf(stderr, " |X"); for (A = 0; A < B.N; A++) fprintf(stderr, " %ld", (long)Xmax[A]);
        fprintf(stderr, "\n");
      }
    }
#endif
#ifdef LLLFP_WALK
    /* check mode: validate the FP walk against the triangular result just
     * computed into _P (which remains the trusted output). */
    if (lf_mode() == 2) lf_check_dim5(&B, X0, Xmax, _P, (long)Cin->index);
#endif
    goto do_sublat;
  }
#endif /* POLY_Dmax >= 5 */

  /* ====== Generic fallback (original algorithm) ====== */
  { Long *x = _P->x[0], xmin[POLY_Dmax], xmax[POLY_Dmax],
         xaux[POLY_Dmax], L, R;
    j = B.n - 1;
    i = Amin[j + 1] - 1;
    R = B.x[j][i];
    xmin[j] = -PD_Floor(X0[i], R);
    xmax[j] = PD_Floor(Xmax[i] - X0[i], R);
    while ((i--) > Amin[j]) {
      Long Low = -X0[i], Upp = Low + Xmax[i];
      R = B.x[B.n - 1][i];
      if (R > 0) {
        if (xmax[j] > (L = PD_Floor(Upp, R))) xmax[j] = L;
        if (xmin[j] < (L = -PD_Floor(-Low, R))) xmin[j] = L;
      } else {
        if (xmax[j] > (L = PD_Floor(-Low, -R))) xmax[j] = L;
        if (xmin[j] < (L = -PD_Floor(Upp, -R))) xmin[j] = L;
      }
    }
    x[j] = xmin[j];
    while (j < B.n) {
      int k;
      if (x[j] > xmax[j]) {
        if (B.n == (++j)) break;
        else x[j]++;
      } else {
        Long Upp = Xmax[i = Amin[j--] - 1], Low = -X0[i];
        int RangeFlag = 0;
        for (k = j + 1; k < B.n; k++)
          Low -= x[k] * B.x[k][i];
        Upp += Low;
        R = B.x[j][i];
        xmin[j] = -PD_Floor(-Low, R);
        xmax[j] = PD_Floor(Upp, R);
        while ((i--) > Amin[j])
          if ((R = B.x[j][i])) {
            Low = -X0[i]; Upp = Xmax[i];
            for (k = j + 1; k < B.n; k++)
              Low -= x[k] * B.x[k][i];
            Upp += Low; R = B.x[j][i];
            if (R > 0) {
              if (xmax[j] > (L = PD_Floor(Upp, R))) xmax[j] = L;
              if (xmin[j] < (L = -PD_Floor(-Low, R))) xmin[j] = L;
            } else {
              if (xmax[j] > (L = PD_Floor(-Low, -R))) xmax[j] = L;
              if (xmin[j] < (L = -PD_Floor(Upp, -R))) xmin[j] = L;
            }
          } else {
            Long X = X0[i];
            for (k = j + 1; k < B.n; k++)
              X += x[k] * B.x[k][i];
            if ((X < 0) || (X > Xmax[i])) RangeFlag = 1;
          }
        if (RangeFlag) ++x[++j];
        else x[j] = xmin[j];
        if (j == 0) {
          while (x[0] <= xmax[0]) {
            Long *y;
            if ((++_P->np) < POINT_Nmax) {
              y = (_P->x[_P->np]);
              for (k = 0; k < B.n; k++) y[k] = x[k];
            } else if (_P->np == POINT_Nmax) {
              y = xaux;
              for (k = 0; k < B.n; k++) y[k] = x[k];
            } else {
              puts("Increase POINT_Nmax"); exit(0);
            }
            x = y; ++x[0];
          }
          x[j = 1]++;
        }
      }
    }
  }

do_sublat:
  if (m)
    CWS_2_SublatZ(_C, &B, &m, M, G);
  if (m)
    Reduce_PPL_2_Sublat(_P, &m, M, G);
}

/*  ==========    Coordinate improvement via CWS-Permutations   ==========  */

typedef struct {
  Long s, g, *N, **G;
  int *p;
} Tri_GLZ_MPaux;
Long Tri_GLZ_Norm(int *d, Long **S) /* sum-norm = default */
{
  Long norm = 0;
  int i, j;
  for (i = 0; i < *d; i++)
    for (j = 0; j < *d; j++)
#ifdef MAX_NORM /* max(|*|) */
    {
      Long x = S[i][j];
      if (x < 0)
        x = -x;
      if (x > norm)
        norm = x;
    }
  return norm;
#else /* sum(|*|) */
    {
      Long x = S[i][j];
      norm += (x > 0) ? x : -x;
    }
  return norm;
#endif
}
void Tri_GLZ_Basis_Perm(int *d, int *pi, /* int *pinv, */ Tri_GLZ_MPaux *AP) {
  Long g, norm, N[AMBI_Dmax], M[AMBI_Dmax][AMBI_Dmax], *S[AMBI_Dmax];
  int i, j; /* if(pi[0]>pi[1]) return; */ /* eliminate equiv. permut. */
  for (i = 0; i < *d; i++) {
    N[i] = AP->N[pi[i]];
    S[i] = M[i];
  }
  g = W_to_GLZ(N, d, S);
  norm = Tri_GLZ_Norm(d, S);
  if (AP->s)
    assert(g == AP->g);
  else
    AP->g = g;
#ifdef TEST_MIN_GLZ
  {
    Long err = 0, max = 0, tsum = 0, pos;
    for (i = 0; i < *d; i++) {
      for (j = 0; j < *d; j++) {
        pos = S[i][j];
        if (pos < 0)
          pos = -pos;
        tsum += pos;
        if (max < pos)
          max = pos;
      }
      if (err == 0) {
        for (j = 0; j < *d; j++)
          err += N[j] * S[i][j];
        if (i == 0)
          err -= g;
      }
    }
    assert((norm == tsum) || (norm == max));
    printf("max=%lld, sum=%lld\n", (long long)max, (long long)tsum);
    if (err) {
      for (i = 0; i < *d; i++)
        printf("%d", pi[i]);
      printf("  g=%d  norm=%d", (int)g, (int)norm);
      puts("");
      for (i = 0; i < *d; i++) {
        printf("S[%d]=", i);
        for (j = 0; j < *d; j++)
          printf(" %5d", S[i][j]);
        if (!i)
          printf("   g=%d  norm=%d", (int)g, (int)norm);
        puts("");
      }
    }
  }
#endif
  if ((0 == AP->s) || (norm < AP->s)) /* init or improve AP->G */
  {
    for (i = 0; i < *d; i++)
      for (j = 0; j < *d; j++)
        AP->G[i][pi[j]] = S[i][j];
    AP->s = norm;
    for (i = 0; i < *d; i++)
      AP->p[i] = pi[i];
  }
}

#ifndef NO_COORD_IMPROVEMENT
/* improved by permutation pi=pinv^-1 */
Long Wperm_to_GLZ(Long *W, int *d, Long **G, int *P) {
  Tri_GLZ_MPaux AS;
  int i, j, pi[AMBI_Dmax], pinv[AMBI_Dmax];
  AS.s = 0;
  AS.N = W;
  AS.G = G;
  AS.p = P;
  Map_Permut(d, pi, /* pinv,*/ (Tri_GLZ_Basis_Perm), (void *)&AS);
  for (i = 0; i < *d; i++)
    for (j = 0; j < *d; j++)
      G[i][j] = AS.G[i][j];
#ifdef TEST_MIN_GLZ
  for (i = 0; i < *d; i++) {
    printf("  G[%d]=", i);
    for (j = 0; j < *d; j++)
      printf(" %5d", G[i][j]);
    if (!i)
      printf("  g=%d norm=%d", (int)AS.g, (int)AS.s);
    puts("");
  }
#endif
  return AS.g;
}
void CWS_to_PermCWS(CWS *Cin, CWS *C, int *pi) {
  int i, j, N = C->N = Cin->N, n = C->nw = Cin->nw, l = 0, A[AMBI_Dmax];
  Long *X, *G[AMBI_Dmax], M[AMBI_Dmax][AMBI_Dmax], W[AMBI_Dmax];
  for (j = 1; j < n; j++)
    if (Cin->d[j] > Cin->d[l])
      l = j;
  X = Cin->W[l];
  j = 0;
  for (i = 0; i < N; i++)
    if (X[i]) {
      A[j] = i;
      W[j] = labs(X[i]);
      G[j] = M[j];
      j++;
    }
  Wperm_to_GLZ(W, &j, G, pi);
  for (i = 0; i < j; i++)
    pi[i] = A[pi[i]];
  for (i = 0; i < N; i++)
    if (X[i] == 0)
      pi[j++] = i;
  C->d[0] = Cin->d[l];
  for (i = 0; i < N; i++)
    C->W[0][i] = Cin->W[l][pi[i]];
  for (j = 1; j < n; j++) {
    int L = j - (j <= l);
    C->d[j] = Cin->d[L];
    for (i = 0; i < N; i++)
      C->W[j][i] = Cin->W[L][pi[i]];
  }
  C->nz = Cin->nz;
  for (j = 0; j < C->nz; j++) {
    C->m[j] = Cin->m[j];
    for (i = 0; i < N; i++)
      C->z[j][i] = Cin->z[j][pi[i]];
  }
}
#endif

/*  ==========        For WS for 5d-polytopes, Sep 2017         ==========  */
int int_ld(Long w) {
  int i = -1;
  while (w) {
    w /= 2;
    i++;
  }
  return i;
}

void Initialize_C5S(C5stats *_C5S, int n) {
  int k;
  if (n < 5) {
    puts("Option '-Q' requires POLY_Dmax > 4!");
    exit(0);
  };
  _C5S->n_nonIP = 0;
  _C5S->n_IP_nonRef = 0;
  _C5S->n_ref = 0;
  _C5S->nr_max_mp = 0;
  _C5S->nr_max_mv = 0;
  _C5S->nr_max_nv = 0;
  _C5S->nr_max_w = 0;
  for (k = 0; k < MAXLD; k++) {
    _C5S->n_w[k] = 0;
    _C5S->nr_n_w[k] = 0;
  }
  _C5S->max_mp = 0;
  _C5S->max_mv = 0;
  _C5S->max_np = 0;
  _C5S->max_nv = 0;
  _C5S->max_h22 = 0;
  _C5S->max_w = 0;
  for (k = 1; k < n - 1; k++)
    _C5S->max_h1[k] = 0;
  for (k = 0; k <= n; k++)
    _C5S->max_nf[k] = 0;
  _C5S->max_chi = -100000000;
  _C5S->min_chi = 100000000;
}

void Update_C5S(BaHo *_BH, int *nf, Long *W, C5stats *_C5S) {
  assert(POLY_Dmax > 4);
  if (_BH->np) { // reflexive case
    int i, chi = 48 + 6 * (_BH->h1[1] - _BH->h1[2] + _BH->h1[3]),
           ld = int_ld(W[5]);
    assert(0 <= ld);
    assert(ld < MAXLD);
    if (_BH->mp > _C5S->max_mp)
      _C5S->max_mp = _BH->mp;
    if (_BH->mv > _C5S->max_mv)
      _C5S->max_mv = _BH->mv;
    if (_BH->np > _C5S->max_np)
      _C5S->max_np = _BH->np;
    if (_BH->nv > _C5S->max_nv)
      _C5S->max_nv = _BH->nv;
    if (_BH->h22 > _C5S->max_h22)
      _C5S->max_h22 = _BH->h22;
    for (i = 1; i < _BH->n - 1; i++)
      if (_BH->h1[i] > _C5S->max_h1[i])
        _C5S->max_h1[i] = _BH->h1[i];
    for (i = 0; i <= _BH->n; i++)
      if (nf[i] > _C5S->max_nf[i])
        _C5S->max_nf[i] = nf[i];
    if (chi > _C5S->max_chi)
      _C5S->max_chi = chi;
    if (chi < _C5S->min_chi)
      _C5S->min_chi = chi;
    _C5S->n_ref++;
    _C5S->n_w[ld]++;
    if (W[5] > _C5S->max_w)
      _C5S->max_w = W[5];
  } else { // IP but non-reflexive case
    if (_BH->mp > _C5S->nr_max_mp)
      _C5S->nr_max_mp = _BH->mp;
    if (_BH->mv > _C5S->nr_max_mv)
      _C5S->nr_max_mv = _BH->mv;
    if (_BH->nv > _C5S->nr_max_nv)
      _C5S->nr_max_nv = _BH->nv;
    _C5S->n_IP_nonRef++;
    _C5S->nr_n_w[int_ld(W[5])]++;
    if (W[5] > _C5S->nr_max_w)
      _C5S->nr_max_w = W[5];
  }
}

void Print_C5S(C5stats *_C5S) {
  int i;
  assert(POLY_Dmax > 4);
  printf("non-IP: #=%ld\n", _C5S->n_nonIP);
  printf(
      "IP, non-reflexive: #=%ld, max_mp=%d, max_mv=%d, max_nv=%d, max_w=%ld\n",
      _C5S->n_IP_nonRef, _C5S->nr_max_mp, _C5S->nr_max_mv, _C5S->nr_max_nv,
      _C5S->nr_max_w);
  printf("  #(w5) of given ld: ");
  for (i = 0; i < MAXLD; i++)
    printf(" %d:%ld", i, _C5S->nr_n_w[i]);
  puts("");
  printf("reflexive: #=%ld, max_mp=%d, max_mv=%d, max_np=%d, max_nv=%d, "
         "max_w=%ld\n",
         _C5S->n_ref, _C5S->max_mp, _C5S->max_mv, _C5S->max_np, _C5S->max_nv,
         _C5S->max_w);
  printf("  #(w5) of given ld: ");
  for (i = 0; i < MAXLD; i++)
    printf(" %d:%ld", i, _C5S->n_w[i]);
  puts("");
  printf("  max #(faces): %d %d %d %d %d\n", _C5S->max_nf[0], _C5S->max_nf[1],
         _C5S->max_nf[2], _C5S->max_nf[3], _C5S->max_nf[4]);
  printf("  h11<=%d, h12<=%d, h13<=%d, h22<=%d, %d<=chi<=%d\n", _C5S->max_h1[1],
         _C5S->max_h1[2], _C5S->max_h1[3], _C5S->max_h22, _C5S->min_chi,
         _C5S->max_chi);
}
