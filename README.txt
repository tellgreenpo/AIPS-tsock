Mateo Mille
tsock


Pour generer l'executable : gcc tsock.v2.c -o [nom desire de l'executable]


====== UDP ==========================================

Compilation sans soucis au niveau de la creation de socket et binding pour recepteur
Compilation sans soucis au niveau de la creation de socket local et adresse distante
pour l'emetteur
Sendto() est fini
recvfrom() est fini
Le programme est fonctionnel on reçoit correctement les meessages
destruction des sockets S et R
Fonctionnement sans soucis, reception sur boucle infinie

===== TCP =========================================================

Creation du socket et adresse distante emmetteur pour TCP
Creation socket et adresse et bind pour cote serveur
Echange de donnes cote serveur et cote emmetteur
Fermeture de la connexion
destruction des sockets S et R
Fonctionement sans soucis

La version 3 a l'air de marcher pas eu le temps de faire la quatre
