# MYMOULETTE 

## AUTHORS   
quentin.rebergue (groupe 40)

## Paliers

[x] Palier0 : Bonnes pratiques, propreté et architecture (3 points)
[x] Palier1 : Restreindre l’environnement (2 points)
[x] Palier 2 : Réduire les capabilities (2 points)
[x] Palier bonus : Utilisable par un utilisateur (2 points)
[x] Palier 3 : Isolation du pauvre (1 point)
[x] Palier 4 : seccomp (2 points)
[x] Palier 5 : Automatisation de la création de l’environnement (1 point)
[x] Palier 6 : Une vraie isolation (2 points)
[x] Palier bonus : Révolution des utilisateurs (2 points)
[x] Palier 7 : Empêcher les fuites d’information (2 points)
[x] Palier 7 bis : Volume attaché au code à moulinéter (1 point)
[x] Palier 8 : Identification du conteneur (1 point)
[x] Palier 9 : pivot_root (2 points)
[ ] Palier 10 : Bac à sable connecté (3 points)

## Makefile

### make
```
make
```
Utilise une image docker qui va compiler le projet avec une regle du Makefile :
`build_bin`. A noter que l'architecture de l'endroit ou on execute le make est
pris en compte ce qui permet d'etre sur que ca soit compatible. (pour ma part
je suis sur ARM).

### make debug
```
make debug
```
Meme principe que `make` sauf qu'il prend l'image Dockerfile.test qui rajoute
les flags `-g` et `--fsanitize=address`

### make test
```
make test
```
Lance la testsuit pour tous les paliers.

### make bonus
```
make bonus
```
Permet de setcap notre binaire avec cap_dac_override pour setup les cgroups

### make clean
```
make clean
```
Supprime le dossier `/out` avec le binaire

### make failrendu
```
make failrendu
```
Build avec les flags de debug, setcap le binaire, execute la testsuite et si
tous les tests passent alors ca clean et creer l'archive de rendu.

## Projet

### Pivot root vs chroot
J'ai mis a disposition une macro dans `src/namespace/setup_rootfs.c` qui
s'appelle `USE_PIVOT_ROOT`:
- `USE_PIVOT_ROOT` = 1 : le programme utilise un pivot root (palier9)
- `USE_PIVOT_ROOT` = 0 : le programme utilise un chroot (palier3)

### Structure du projet

`src/child`:
    - Fonction qui setup le process enfant a isoler
    - Fonction entry point de l'enfant
`src/controller`
    - Fonction pour setup les cgroups (palier 1)
`src/isolation`
    - Fonction pour set les capabilities (palier 2)
    - Fonction pour set les seccomps (palier 4)
`src/namespace`
    - Fonction pour unmout et mount les FS (palier 7 et 7bis)
    - Fonction pour chroot ou pivot root (palier 3 et 9)
    - Fonction pour setup les namespaces (palier 6 et bonus)
`src/request`
    - Fonction pour faire des requetes a docker (palier 4)
    - Fonction pour parser et extraire le JSON (palier 4)
    - Fonction pour build l'image (palier 4)
`src/utils`
    - Fonction pour parser les arguments
    - Fonction pour write dans des files
    - Autre fonction utile

## Conclusion

J'ai beaucoup aime le projet. Je suis tres satisfait du travail que j'ai mene
seul. Le palier 5 est assez long et penible surtout pour 1 point mais c'est
celui qui permet vraiment que notre projet ressemble a un conteneur.
Attention aux tests qui sont assez gourmand en pull d'image. J'ai deja plusieurs
fois depasse la rate limit.

J'ai essaye de faire le code le plus clean possible, c'est quelque chose que
j'adore faire.

J'aurais bien voulu faire une partie de tests qui essaient vraiment de casser
le programme mais je suis limite en temps...

J'ai tente de cree une autre image docker pour faire les tests dedans mais il
y avait pleins de soucis vu que c'etait dans un environnement virtualise.

# myDocker
