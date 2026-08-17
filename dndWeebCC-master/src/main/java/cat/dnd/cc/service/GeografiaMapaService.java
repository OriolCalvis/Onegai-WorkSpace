package cat.dnd.cc.service;

import cat.dnd.cc.model.GeografiaMapa;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Map;

/**
 * Lee y escribe data/mapa/geografia.json (polígonos de nación, marcadores de facción y
 * puntos personalizados del Mapa Mundi). Vive aparte de MapaController porque
 * AventuraController también necesita leerla (para saber en qué facciones/puntos aparece
 * una aventura y enlazar de vuelta al mapa).
 */
@Service
public class GeografiaMapaService {

    private static final Path GEOGRAFIA_JSON = Paths.get("data", "mapa", "geografia.json");

    private final ObjectMapper objectMapper;

    public GeografiaMapaService(ObjectMapper objectMapper) {
        this.objectMapper = objectMapper;
    }

    /** Valores de respaldo si data/mapa/geografia.json no existe o no se puede leer,
     * para que el mapa no se rompa (aunque salga sin polígonos de nación ajustados). */
    private GeografiaMapa geografiaPorDefecto() {
        GeografiaMapa g = new GeografiaMapa();
        Map<String, int[]> m = g.getMarcadores();
        m.put("cofradia_de_mascaras_de_plata", new int[]{560, 865});
        m.put("clan_de_gongorguma_renegado", new int[]{1290, 370});
        m.put("milicia_corrupta_de_llerba", new int[]{600, 720});
        m.put("manada_del_rei_llop", new int[]{640, 600});
        m.put("sagas_de_la_luna_hueca", new int[]{650, 560});
        m.put("corte_de_espinas", new int[]{280, 780});
        m.put("bandidos_del_camino_de_ceniza", new int[]{250, 900});
        m.put("espectros_de_la_tomba", new int[]{300, 1000});
        m.put("nagas_del_pozo_azul", new int[]{480, 1000});
        m.put("plaga_de_san_lazaro", new int[]{620, 1000});
        m.put("renacidos_de_la_fosa", new int[]{650, 900});
        m.put("yokai_del_umbral", new int[]{580, 800});
        m.put("hechiceros_del_vacio", new int[]{500, 750});
        m.put("forjados_sin_amo", new int[]{650, 750});
        m.put("horrores_del_cielo_fragmentado", new int[]{400, 850});
        return g;
    }

    public GeografiaMapa cargar() {
        if (Files.exists(GEOGRAFIA_JSON)) {
            try {
                return objectMapper.readValue(GEOGRAFIA_JSON.toFile(), GeografiaMapa.class);
            } catch (IOException e) {
                System.err.println("No se pudo leer " + GEOGRAFIA_JSON + ": " + e.getMessage());
            }
        }
        return geografiaPorDefecto();
    }

    public void guardar(GeografiaMapa geografia) throws IOException {
        Files.createDirectories(GEOGRAFIA_JSON.getParent());
        objectMapper.writerWithDefaultPrettyPrinter().writeValue(GEOGRAFIA_JSON.toFile(), geografia);
    }
}
