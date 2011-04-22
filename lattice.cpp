#include "lattice.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* ----------------------------------------------------------------------
   constructor does nothing
------------------------------------------------------------------------- */
lattice::lattice()
{
  initialized = 0;
  nlayer = nucell = ntype = 0;

  name = NULL;
  attyp = NULL;
  atpos = NULL;

  layer = NULL;
  numlayer = NULL;
  h        = NULL;

  memory = new Memory;
}

/* ----------------------------------------------------------------------
   deconstructor destroy any allocated memory
------------------------------------------------------------------------- */
lattice::~lattice()
{
  if (atpos) memory->destroy(atpos);
  if (attyp) delete []attyp;
  if (name ) delete []name;
  if (layer) delete []layer;
  if (numlayer) delete []numlayer;
  if (h       ) delete []h;
  
  delete memory;
}

/* ----------------------------------------------------------------------
   Display lattice basic info
------------------------------------------------------------------------- */
void lattice::display()
{
  if (!initialized) return;
  printf("\n======================= Lattice info ======================\n");
  printf("Lattice name......................: %s\n", name);
  printf("Lattice constant of unit cell.....: %g\n", alat);
  printf("Number of atom per unit cell......: %d\n", nucell);
  printf("Number of atom types per unit cell: %d\n", ntype);
  printf("-----------------------------------------------------------\n");
  printf("Lattice vectors:\n");
  for (int i=0; i<3; i++){
    for(int j=0; j<3; j++) printf("%lf ", latvec[i][j]);
    printf("\n");
  }
  printf("-----------------------------------------------------------\n");
  printf("Basis (Fractional coordinate):\n");
  for (int i=0; i<nucell; i++){
    printf("%lf %lf %lf\n", atpos[i][0], atpos[i][1], atpos[i][2]);
  }
  printf("===========================================================\n\n");
}

/*------------------------------------------------------------------------------
 * Method to re-orient the lattice, hope to follow the rules of LAMMPS
 *----------------------------------------------------------------------------*/
void lattice::OrientLattice()
{
  double LV[3][3], box[3], oldy[3];
  box[0] = box[1] = box[2] = 0.;
  for (int i=0; i<3; i++){
    for (int j=0; j<3; j++){
      LV[i][j] = latvec[i][j];
      box[i] += latvec[i][j]*latvec[i][j];
      latvec[i][j] = 0.;
    }
    box[i] = sqrt(box[i]);
  }
  // new A
  latvec[0][0] = box[0];
  // new B
  for (int i=0; i<3; i++) latvec[1][0] += LV[0][i]*LV[1][i]/box[0];
  latvec[1][1] = sqrt(box[1]*box[1]-latvec[1][0]*latvec[1][0]);
  // new C
  for (int i=0; i<3; i++) latvec[2][0] += LV[0][i]*LV[2][i]/box[0];

  for (int i=0; i<3; i++) oldy[i] = LV[1][i] - latvec[1][0]*LV[0][i]/box[0];
  double yl=0.;
  for (int i=0; i<3; i++) yl += oldy[i]*oldy[i];
  latvec[2][1] = (oldy[0]*LV[2][0] + oldy[1]*LV[2][1] + oldy[2]*LV[2][2])/sqrt(yl);
  latvec[2][2] = sqrt(box[2]*box[2]-latvec[2][0]*latvec[2][0]-latvec[2][1]*latvec[2][1]);

return;
}

/*------------------------------------------------------------------------------
 * Method to count # of words in a string, without destroying the string
 *----------------------------------------------------------------------------*/
int lattice::count_words(const char *line)
{
  int n = strlen(line) + 1;
  char *copy = (char *) memory->smalloc(n*sizeof(char),"copy");
  strcpy(copy,line);

  char *ptr;
  if (ptr = strchr(copy,'#')) *ptr = '\0';

  if (strtok(copy," \t\n\r\f") == NULL) {
    memory->sfree(copy);
    return 0;
  }
  n = 1;
  while (strtok(NULL," \t\n\r\f")) n++;

  memory->sfree(copy);
  return n;
}

/*------------------------------------------------------------------------------
 * Method to setup the dependent parameters
 *----------------------------------------------------------------------------*/
void lattice::setup()
{
  if (initialized == 0) return;
  nlayer = 0;
  for (int i=0; i<nucell; i++) nlayer = MAX(nlayer, layer[i]);
  nlayer++;

  if (h) delete []h;
  if (numlayer) delete []numlayer;
  h = new double[nlayer];
  numlayer = new int[nlayer];

  for (int i=0; i<nlayer; i++) numlayer[i] = 0;
  for (int i=0; i<nucell; i++) numlayer[layer[i]]++;

  h[0] = 0.;
  int i0 = 0;
  double pos0[3], pos1[3];
  for (int i=0; i<nucell; i++) if (layer[i] == 0) {i0 = i; break;}
  pos0[0] = pos0[1] = pos0[2] = 0.;
  for (int i=0; i<3; i++)
  for (int j=0; j<3; j++) pos0[j] += atpos[i0][i] * latvec[i][j];

  for (int il=1; il<nlayer; il++){
    int i1;
    for (int i=0; i<nucell; i++) if (layer[i] == il) {i1 = i; break;}
    for (int i=0; i<3; i++)
    for (int j=0; j<3; j++) pos1[j] += atpos[i1][i] * latvec[i][j];
    h[il] = (pos1[2] - pos0[2])*alat;
  }

return;
}

