/**
 * Models serialitzables del domini.
 *
 * <p>Els models son POJOs pensats per Jackson i Thymeleaf: camps, getters/setters i
 * petits helpers de presentacio. Les regles que combinen diverses cartes o validen
 * restriccions viuen als serveis. Les cartes d'aventura per actes ({@code CartaAventura},
 * {@code CardKind} i {@code FichaEstado}) formen part del model narratiu nou i conviuen
 * amb les aventures heretades.</p>
 */
package cat.dnd.cc.model;
