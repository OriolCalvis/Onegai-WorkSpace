package cat.dnd.cc.service;

import cat.dnd.cc.eines.mazo.CartaFisica;
import cat.dnd.cc.eines.mazo.MazoDeClase;
import cat.dnd.cc.eines.mazo.PdfCartas;
import cat.dnd.cc.model.TierClass;
import cat.dnd.cc.model.TierEquipment;
import cat.dnd.cc.model.TierPassive;
import cat.dnd.cc.model.TierSkill;
import cat.dnd.cc.model.TierSpell;
import cat.dnd.cc.model.TierSummon;
import cat.dnd.cc.repository.TierClassRepository;
import cat.dnd.cc.repository.TierEquipmentRepository;
import cat.dnd.cc.repository.TierPassiveRepository;
import cat.dnd.cc.repository.TierSkillRepository;
import cat.dnd.cc.repository.TierSpellRepository;
import cat.dnd.cc.repository.TierSummonRepository;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.stereotype.Service;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * Los mazos físicos de las clases: las cartas de cada clase listas para imprimir y jugar en
 * la mesa, en PDF (una clase o varias) o en un ZIP con un PDF por clase.
 * <p>
 * CARTAS-CLASES-01 (08/10/2026). La lógica del mazo vive en {@link MazoDeClase} (sin Spring) y
 * el PDF en {@link PdfCartas}; aquí solo se buscan las cartas en los repositorios y se leen las
 * cartas de reglas de mesa de {@code data/reglas/referencia_mesa.json}.
 */
@Service
public class MazoClaseService {

    private static final Path REGLAS_DE_MESA = Paths.get("data", "reglas", "referencia_mesa.json");

    private final TierClassRepository clases;
    private final TierSkillRepository habilidades;
    private final TierSpellRepository hechizos;
    private final TierPassiveRepository pasivas;
    private final TierSummonRepository invocaciones;
    private final TierEquipmentRepository equipo;
    private final ObjectMapper objectMapper;

    public MazoClaseService(TierClassRepository clases, TierSkillRepository habilidades,
                            TierSpellRepository hechizos, TierPassiveRepository pasivas,
                            TierSummonRepository invocaciones, TierEquipmentRepository equipo,
                            ObjectMapper objectMapper) {
        this.clases = clases;
        this.habilidades = habilidades;
        this.hechizos = hechizos;
        this.pasivas = pasivas;
        this.invocaciones = invocaciones;
        this.equipo = equipo;
        this.objectMapper = objectMapper;
    }

    /** Las cartas del mazo de una clase, en el orden en que se imprimen. */
    public List<CartaFisica> mazo(TierClass clase, MazoDeClase.Opciones opciones) {
        return MazoDeClase.construir(clase, catalogo(), opciones, opciones.reglas() ? reglasDeMesa() : List.of());
    }

    public byte[] pdf(TierClass clase, MazoDeClase.Opciones opciones) {
        return PdfCartas.generar("Mazo de " + clase.getName(), mazo(clase, opciones));
    }

    /** Un solo PDF con los mazos de varias clases; cada una empieza en hoja nueva. */
    public byte[] pdfDeVarias(List<TierClass> lista, MazoDeClase.Opciones opciones) {
        MazoDeClase.Catalogo cat = catalogo();
        List<CartaFisica> reglas = opciones.reglas() ? reglasDeMesa() : List.of();
        List<PdfCartas.Mazo> mazos = new ArrayList<>();
        for (TierClass c : lista) {
            mazos.add(new PdfCartas.Mazo("Mazo de " + c.getName(), MazoDeClase.construir(c, cat, opciones, reglas)));
        }
        return PdfCartas.generar(mazos);
    }

    /** Un ZIP con un PDF por clase ({@code mazo-<id>.pdf}). */
    public byte[] zip(List<TierClass> lista, MazoDeClase.Opciones opciones) {
        MazoDeClase.Catalogo cat = catalogo();
        List<CartaFisica> reglas = opciones.reglas() ? reglasDeMesa() : List.of();
        ByteArrayOutputStream salida = new ByteArrayOutputStream();
        try (ZipOutputStream zip = new ZipOutputStream(salida)) {
            for (TierClass c : lista) {
                byte[] pdf = PdfCartas.generar("Mazo de " + c.getName(), MazoDeClase.construir(c, cat, opciones, reglas));
                zip.putNextEntry(new ZipEntry("mazo-" + c.getId() + ".pdf"));
                zip.write(pdf);
                zip.closeEntry();
            }
        } catch (IOException e) {
            throw new UncheckedIOException("No se pudo crear el ZIP de mazos", e);
        }
        return salida.toByteArray();
    }

    /** Las clases de una familia ("todas" o vacío = todas), en el orden del álbum. */
    public List<TierClass> clasesDeFamilia(String familia) {
        List<TierClass> todas = clases.findAll();
        if (familia == null || familia.isBlank() || familia.equals("todas")) {
            return todas;
        }
        return todas.stream().filter(c -> MazoDeClase.claveFamilia(c).equals(familia)).toList();
    }

    /** La clave de familia de cada clase (id -> "sp-mago", "propia"...), para el álbum. */
    public Map<String, String> familiaPorClase() {
        Map<String, String> salida = new HashMap<>();
        for (TierClass c : clases.findAll()) {
            salida.put(c.getId(), MazoDeClase.claveFamilia(c));
        }
        return salida;
    }

    /** Cuántas clases hay de cada familia, en el orden en que se enseñan (sin las vacías). */
    public Map<String, Integer> recuentoPorFamilia() {
        Map<String, Integer> cuenta = new LinkedHashMap<>();
        for (String clave : MazoDeClase.FAMILIAS.keySet()) cuenta.put(clave, 0);
        for (TierClass c : clases.findAll()) cuenta.merge(MazoDeClase.claveFamilia(c), 1, Integer::sum);
        cuenta.values().removeIf(n -> n == 0);
        return cuenta;
    }

    /**
     * Las cartas de reglas de mesa (data/reglas/referencia_mesa.json). Se leen cada vez: son
     * cinco cartas y así un cambio en el fichero se ve sin reiniciar. Si el fichero no está o
     * no se puede leer, el mazo sale sin ellas (no es motivo para no imprimir).
     */
    public List<CartaFisica> reglasDeMesa() {
        List<CartaFisica> cartas = new ArrayList<>();
        if (!Files.exists(REGLAS_DE_MESA)) {
            return cartas;
        }
        try {
            JsonNode raiz = objectMapper.readTree(REGLAS_DE_MESA.toFile());
            for (JsonNode n : raiz.path("cartas")) {
                CartaFisica c = new CartaFisica(CartaFisica.Tipo.REGLA, n.path("titulo").asText(""))
                        .id(n.path("id").asText(""))
                        .tier(0)
                        .linea(n.path("linea").asText("Reglas de mesa · Ed. 4"))
                        .texto(n.path("texto").asText(""))
                        .pie("Reglamento Ed. 4 · Sistema_Cartas_Tiers.md");
                for (JsonNode d : n.path("datos")) {
                    c.dato(d.asText(""));
                }
                cartas.add(c);
            }
        } catch (IOException e) {
            System.err.println("No se pudieron leer las reglas de mesa " + REGLAS_DE_MESA + ": " + e.getMessage());
        }
        return cartas;
    }

    /** Las cartas, con índices para las invocaciones (summonedBy) y los sets (grantedTags). */
    private MazoDeClase.Catalogo catalogo() {
        Map<String, List<TierSummon>> porCarta = new HashMap<>();
        for (TierSummon s : invocaciones.findAll()) {
            if (s.getSummonedBy() != null) {
                porCarta.computeIfAbsent(s.getSummonedBy(), k -> new ArrayList<>()).add(s);
            }
        }
        Map<String, List<TierEquipment>> porSet = new HashMap<>();
        for (TierEquipment e : equipo.findAll()) {
            for (String t : e.getGrantedTags()) {
                if (t != null && t.startsWith("Set ")) {
                    porSet.computeIfAbsent(t, k -> new ArrayList<>()).add(e);
                }
            }
        }
        // Las piezas del set en el orden del cuerpo: arma, cabeza, torso, piernas, pies
        List<String> orden = List.of("MainHand", "arma_principal", "OffHand", "arma_secundaria",
                "Head", "cabeza", "Torso", "torso", "Legs", "piernas", "Feet", "pies");
        porSet.values().forEach(l -> l.sort((a, b) -> Integer.compare(posicion(orden, a.getSlot()), posicion(orden, b.getSlot()))));

        return new MazoDeClase.Catalogo() {
            @Override
            public TierSkill habilidad(String id) {
                return habilidades.findById(id).orElse(null);
            }

            @Override
            public TierSpell hechizo(String id) {
                return hechizos.findById(id).orElse(null);
            }

            @Override
            public TierPassive pasiva(String id) {
                return pasivas.findById(id).orElse(null);
            }

            @Override
            public TierSummon invocacion(String id) {
                return invocaciones.findById(id).orElse(null);
            }

            @Override
            public TierEquipment equipo(String id) {
                return equipo.findById(id).orElse(null);
            }

            @Override
            public List<TierSummon> invocacionesDe(String cartaId) {
                return porCarta.getOrDefault(cartaId, List.of());
            }

            @Override
            public List<TierEquipment> piezasDelSet(String etiqueta) {
                return porSet.getOrDefault(etiqueta, List.of());
            }
        };
    }

    private static int posicion(List<String> orden, String slot) {
        int i = orden.indexOf(slot);
        return i < 0 ? orden.size() : i;
    }
}
