/*    h_falcon.cpp
 *
 *    Copyright (c) 2008, Jeremy Cowgar
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#ifdef CONFIG_HILIT_FALCON

enum { hsFalcon_Normal, hsFalcon_Comment, hsFalcon_CommentL, hsFalcon_Keyword,
hsFalcon_String1, hsFalcon_String2 };

#define FALCON_COMMENT_DELTA 1

static const char *indentTokens[] = {
   "case",
   "catch",
   "class",
   "default",
   "elif",
   "else",
   "for",
   "function",
   "init",
   "loop",
   "select",
   "switch",
   "try",
   "while"
};

static const char *unindentTokens[] = {
   "end"
};

static const char *lineUnindentTokens[] = {
   "case",
   "catch",
   "default",
   "elif",
   "else"
};

int Hilit_FALCON(EBuffer *BF, int /*LN*/, PCell B, int Pos, int Width, ELine *Line, hlState &State, hsState *StateMap, int *ECol) {
   int j = 0;
   int firstnw = 0;
   HILIT_VARS(BF->Mode->fColorize->Colors, Line);

   C = 0;
   NC = 0;

   for(i = 0; i < Line->Count;) {
      if (*p != ' ' && *p != 9) firstnw++;
      IF_TAB() else {
         switch(State) {
         case hsFalcon_Comment:
            Color = CLR_Comment;
            // Is the next 2 chars */ ?
            if ((len >= 2) && (*p == '*') && (p[1] == '/')) {
               ColorNext();
            set_normal:
               ColorNext();
               State = hsFalcon_Normal;
               continue;
            }
            goto hilit;
         case hsFalcon_String1:
            Color = CLR_String;
            if (*p == '\\' && (*(p+1) == '\'') )  {
               ColorNext();
            } else if (*p == '\'') {
               goto set_normal;
            }
            goto hilit;
         case hsFalcon_String2:
            Color = CLR_String;
            if (*p == '\\' && (*(p+1) == '"') ) {
               ColorNext();
            } else if (*p == '"') { // TODO: do we need to trap \" ?
               goto set_normal;
            }
            goto hilit;
         default:
         case hsFalcon_Normal:
            if (isalpha(*p) || (*p == '_')) {
               j = 0;
               while (((i + j) < Line->Count) &&
                  (isalnum(Line->Chars[i+j]) ||
                   (Line->Chars[i + j] == '_'))
                      ) j++;
               if (BF->GetHilitWord(j, Line->Chars + i, Color, 1)) {
                  State = hsFalcon_Keyword;
               } else {
                  int x;
                  x = i + j;
                  if ((x < Line->Count) && (Line->Chars[x] == '(')) {
                     Color = CLR_Function;
                  } else {
                     Color = CLR_Normal;
                  }
                  State = hsFalcon_Normal;
               }
               if (StateMap)
                  memset(StateMap + i, State, j);
               if (B)
                  MoveMem(B, C - Pos, Width, Line->Chars + i, HILIT_CLRD(), j);
               i += j;
               len -= j;
               p += j;
               C += j;
               State = hsFalcon_Normal;
               continue;
            } else if ((len >= 2) && ( (*p == '/') && (*(p+1) == '*') )) {
               State = hsFalcon_Comment;
               Color = CLR_Comment;
            hilit2:
               ColorNext();
            hilit:
               ColorNext();
               continue;
            } else if ((len >= 2) && (*p == '/') && (p[1] == '/')) {
               State = hsFalcon_CommentL;
               Color = CLR_Comment;
               goto hilit2;
               continue;
            } else if (isdigit(*p)) {
               Color = CLR_Number;
               ColorNext();
               while(len && isdigit(*p)) ColorNext();
               continue;
            } else if (*p == '\'') {
               State = hsFalcon_String1;
               Color = CLR_String;
               goto hilit;
            } else if (*p == '"') {
               State = hsFalcon_String2;
               Color = CLR_String;
               goto hilit;
            } else if (ispunct(*p) && *p != '_') {
               Color = CLR_Punctuation;
               goto hilit;
            }
            Color = CLR_Normal;
            goto hilit;
         case hsFalcon_CommentL:
            Color = CLR_Comment;
            goto hilit;
         }
      }
   }
   if (State == hsFalcon_CommentL)
      State = hsFalcon_Normal;
   *ECol = C;
   return 0;
}

#ifdef CONFIG_INDENT_FALCON

int Falcon_Base_Indent = 4;
int Falcon_Paren_Delta = 2;
int Falcon_Max_Paren   = 40;

#define FALCON_BASE_INDENT     Falcon_Base_Indent
#define FALCON_PAREN_DELTA     Falcon_Paren_Delta
#define FALCON_MAX_PAREN       Falcon_Max_Paren

#define FCTX_INDENT  1
#define FCTX_OUTDENT 2
#define FCTX_PAREN   3
#define FCTX_BRACKET 4

static int FMatch(int Len, int Pos, hsState *StateMap, const char *Text, const char *String, hsState State) {
    int L = strlen(String);

    if (Pos + L <= Len)
        if (StateMap == NULL || IsState(StateMap + Pos, State, L))
            if (strnicmp(String, Text + Pos, L) == 0)
                return 1;
    return 0;
}

static int FSearchMatch(int Count, EBuffer *B, int Row, int Ctx) {
   char *P;
   int L;
   int Pos;
   int StateLen;
   hsState *StateMap;
   int ICount = (Ctx == 2) ? Count : 0;
   
   Count = (Ctx == 2) ? 0 : Count;
   
   while (Row >= 0) {
      P = B->RLine(Row)->Chars;
      L = B->RLine(Row)->Count;
      StateMap = NULL;
      if (B->GetMap(Row, &StateLen, &StateMap) == 0) return -1;
      Pos = L - 1;
      if (L > 0) while (Pos >= 0) {
         if (Ctx == FCTX_PAREN && FMatch(L, Pos, StateMap, P, "(", hsFalcon_Normal)) {
            Count--;
         retpos:
            if (Count == 0) {
               if (StateMap)
                  free(StateMap);
               return Pos;
            }
         }
         else if (Ctx == FCTX_PAREN && FMatch(L, Pos, StateMap, P, ")", hsFalcon_Normal)) {
            Count++;
            goto retpos;
         }
         else if (Ctx == FCTX_BRACKET && FMatch(L, Pos, StateMap, P, "[", hsFalcon_Normal)) {
            Count--;
            goto retpos;
         }
         else if (Ctx == FCTX_BRACKET && FMatch(L, Pos, StateMap, P, "]", hsFalcon_Normal)) {
            Count++;
            goto retpos;
         }
         else if (isalpha(P[Pos])) {
            if (Ctx == FCTX_INDENT) {
               if (FMatch(L, Pos, StateMap, P, "if", hsFalcon_Keyword) &&
                   P[Pos-2] != 'e' && P[Pos-1] != 'l')
               {
                  int tPos = Pos;
                  int oneLiner = 0;
                  // TODO: this does not take into account strings that
                  //       may contain :
                  for (int tPos = Pos; tPos < L; tPos++) {
                     if (P[tPos] == ':') {
                        oneLiner = 1;
                        break;
                     }
                  }
   
                  if (!oneLiner) {
                     Count++;
                  }

                  if (Count == 0) {
                     if (StateMap)
                        free(StateMap);
                     return B->LineIndented(Row) + FALCON_BASE_INDENT;
                  }
               }
               else if (FMatch(L, Pos, StateMap, P, "function", hsFalcon_Keyword) ||
                   FMatch(L, Pos, StateMap, P, "class", hsFalcon_Keyword) ||
                   FMatch(L, Pos, StateMap, P, "while", hsFalcon_Keyword) ||
                   FMatch(L, Pos, StateMap, P, "loop", hsFalcon_Keyword) ||
                   FMatch(L, Pos, StateMap, P, "init", hsFalcon_Keyword) ||
                   FMatch(L, Pos, StateMap, P, "for", hsFalcon_Keyword) ||
                   FMatch(L, Pos, StateMap, P, "select", hsFalcon_Keyword) ||
                   FMatch(L, Pos, StateMap, P, "switch", hsFalcon_Keyword) ||
                   FMatch(L, Pos, StateMap, P, "try", hsFalcon_Keyword))
               {
                  Count++;
                  if (Count == 0) {
                     if (StateMap)
                        free(StateMap);
                     return B->LineIndented(Row) + FALCON_BASE_INDENT;
                  }
               } else if (FMatch(L, Pos, StateMap, P, "else", hsFalcon_Keyword) ||
                          FMatch(L, Pos, StateMap, P, "elif", hsFalcon_Keyword) ||
                          FMatch(L, Pos, StateMap, P, "case", hsFalcon_Keyword) ||
                          FMatch(L, Pos, StateMap, P, "default", hsFalcon_Keyword) ||
                          FMatch(L, Pos, StateMap, P, "catch", hsFalcon_Keyword))
               {
               } else if (FMatch(L, Pos, StateMap, P, "end", hsFalcon_Keyword)) {
                  Count--;
                  if (Count == 0) {
                     if (StateMap)
                        free(StateMap);
                     return B->LineIndented(Row) - FALCON_BASE_INDENT;
                  }
               }
            }
         }
         Pos--;
      }
      if (StateMap)
         free(StateMap);
      Row--;
   }
   return -1;
}

static int FSearchBackContext(EBuffer *B, int Row, char &ChFind, int &paren, int &bracket) {
   char *P;
   int L;
   int Pos;
   int Count = -1;
   int StateLen;
   hsState *StateMap;
   int is_blank = 0;

   paren = 0;
   bracket = 0;

   ChFind = '0';
   while (Row >= 0) {
      P = B->RLine(Row)->Chars;
      L = B->RLine(Row)->Count;
      StateMap = NULL;
      if (B->GetMap(Row, &StateLen, &StateMap) == 0) return 0;
      Pos = L - 1;
      if (L > 0) while (Pos >= 0) {
         if (isalpha(P[Pos]) && (Pos == 0 || !isalpha(P[Pos-1]))) {
            if (FMatch(L, Pos, StateMap, P, "if", hsFalcon_Keyword))
            {
               int tPos = Pos;
               int oneLiner = 0;
               // TODO: this does not take into account strings that
               //       may contain :
               for (int tPos = Pos; tPos < L; tPos++) {
                  if (P[tPos] == ':') {
                     oneLiner = 1;
                     break;
                  }
               }

               if (!oneLiner) {
                  Count++;
                  ChFind = 'i';
               }
            }
            else if (FMatch(L, Pos, StateMap, P, "function", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "class", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "while", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "init", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "loop", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "for", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "select", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "switch", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "try", hsFalcon_Keyword) ||
                FMatch(L, Pos, StateMap, P, "if", hsFalcon_Keyword))
            {
               Count++;
               ChFind = 'i';
            } else if (FMatch(L, Pos, StateMap, P, "else", hsFalcon_Keyword) ||
                       FMatch(L, Pos, StateMap, P, "elif", hsFalcon_Keyword) ||
                       FMatch(L, Pos, StateMap, P, "case", hsFalcon_Keyword) ||
                       FMatch(L, Pos, StateMap, P, "default", hsFalcon_Keyword) ||
                       FMatch(L, Pos, StateMap, P, "catch", hsFalcon_Keyword)) {
               ChFind = 'i';
            } else if (FMatch(L, Pos, StateMap, P, "end", hsFalcon_Keyword)) {
               Count--;
               ChFind = 'o';
            } else {
               is_blank++;
               Pos--;
               continue;
            }
            if (Count == 0) {
               if (StateMap)
                  free(StateMap);
               return B->LineIndented(Row);
            }
         }

         if (FMatch(L, Pos, StateMap, P, "(", hsFalcon_Normal))
            paren++;
         else if (FMatch(L, Pos, StateMap, P, ")", hsFalcon_Normal))
            paren--;
         else if (FMatch(L, Pos, StateMap, P, "[", hsFalcon_Normal))
            bracket++;
         else if (FMatch(L, Pos, StateMap, P, "]", hsFalcon_Normal))
            bracket--;

         if (P[Pos] != ' ' && P[Pos] != 9) is_blank++;
         Pos--;
      }
      if (StateMap)
         free(StateMap);
      Row--;
   }
   return -1;
}

static int FIndentComment(EBuffer *B, int Row, int /*StateLen*/, hsState * /*StateMap*/) {
    int I = 0, R;

    if (Row > 0) {
        R = Row - 1;
        while (R >= 0) {
            if (B->RLine(R)->Count == 0) R--;
            else {
                I = B->LineIndented(R);
                break;
            }
        }
        if (B->RLine(Row - 1)->StateE == hsFalcon_Comment)
           if (LookAt(B, Row - 1, I, "/*", hsFalcon_Comment, 0))
              I += FALCON_COMMENT_DELTA;
    }
    return I;
}

static int FIndentNormal(EBuffer *B, int Line, int /*StateLen*/, hsState * /*StateMap*/) {
   int I = 0;
   int paren = 0;
   int bracket = 0;

   if (LookAt(B, Line, 0, "end", hsFalcon_Keyword)) {
      return FSearchMatch(-2, B, Line - 1, FCTX_INDENT);
   } else if (LookAt(B, Line, 0, "else",    hsFalcon_Keyword) ||
              LookAt(B, Line, 0, "elif",    hsFalcon_Keyword) ||
              LookAt(B, Line, 0, "catch",   hsFalcon_Keyword) ||
              LookAt(B, Line, 0, "case",    hsFalcon_Keyword) ||
              LookAt(B, Line, 0, "default", hsFalcon_Keyword)) {
      return FSearchMatch(-2, B, Line - 1, FCTX_INDENT);
   } else {
      char ChFind;

      I = FSearchBackContext(B, Line - 1, ChFind, paren, bracket);

      // Unmatched parens?
      if (paren) {
         int off = 0;
         int tI = FSearchMatch(1, B, Line - 1, FCTX_PAREN);
         if (!LookAt(B, Line, 0, ")", hsFalcon_Normal))
            off = FALCON_PAREN_DELTA;
         if ( tI < FALCON_MAX_PAREN )
            return tI + off;
         I += FALCON_BASE_INDENT * paren + off;
      }

      // Unmatched brackets?
      if (bracket) {
         int off = 0;
         int tI = FSearchMatch(1, B, Line - 1, FCTX_BRACKET);
         if (!LookAt(B, Line, 0, "]", hsFalcon_Normal))
            off = FALCON_PAREN_DELTA;
         if ( tI < FALCON_MAX_PAREN )
            return tI + off;
         I += FALCON_BASE_INDENT * bracket + off;
      }

      /*
       * Line starts with a quote?
       *
       * vara = "ABC"
       *        "DEF"
       * varb = \
       *        "ABC"
       *        "DEF"
       */

      if (I == -1)
         return 0;
      switch (ChFind) {
      case 'i':
          return I + FALCON_BASE_INDENT;
      case 'o':
         return I - FALCON_BASE_INDENT;
      default:
         return I;
      }
   }
}

int Indent_FALCON(EBuffer *B, int Line, int PosCursor) {
   int I;
   hsState *StateMap = NULL;
   int StateLen = 0;
   int OI;
   
   OI = I = B->LineIndented(Line);
   if (I != 0) 
      B->IndentLine(Line, 0);
   if (B->GetMap(Line, &StateLen, &StateMap) == 0) 
      return 0;
   
   if (StateLen > 0) {
      if (StateMap[0] == hsFalcon_Comment) {
         I = FIndentComment(B, Line, StateLen, StateMap);
      } else {
         I = FIndentNormal(B, Line, StateLen, StateMap);
      }
   } else {
      I = FIndentNormal(B, Line, 0, NULL);
   }
   if (StateMap)
      free(StateMap);
   if (I >= 0)
      B->IndentLine(Line, I);
   else
      I = 0;
   if (PosCursor == 1) {
      int X = B->CP.Col;
      
      X = X - OI + I;
      if (X < I) X = I;
      if (X < 0) X = 0;
      if (X > B->LineLen(Line)) {
         X = B->LineLen(Line);
         if (X < I) X = I;
      }
      if (B->SetPosR(X, Line) == 0)
         return 0;
   } else if (PosCursor == 2) {
      if (B->SetPosR(I, Line) == 0)
         return 0;
   }
   return 1;
}
          
#endif /* CONFIG_INDENT_FALCON */
#endif /* CONFIG_HILIT_FALCON */
