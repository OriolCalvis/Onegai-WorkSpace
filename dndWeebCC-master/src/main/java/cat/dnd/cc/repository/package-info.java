/**
 * Repositoris de persistencia JSON.
 *
 * <p>No hi ha base de dades: cada repositori carrega una carpeta sota {@code data/},
 * mante una cache en memoria i escriu un fitxer JSON per entitat. Aquest patro fa el
 * contingut molt editable, pero exigeix commits frequents abans d'executar generadors.</p>
 */
package cat.dnd.cc.repository;
