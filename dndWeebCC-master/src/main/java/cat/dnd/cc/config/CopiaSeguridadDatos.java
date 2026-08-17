package cat.dnd.cc.config;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.ApplicationArguments;
import org.springframework.boot.ApplicationRunner;
import org.springframework.stereotype.Component;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Comparator;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;
import java.util.stream.Stream;

/**
 * Blindaje de datos (tarea Z2): en CADA arranque, antes de servir peticiones, comprime todo
 * el árbol {@code data/} en un zip con marca de tiempo dentro de {@code data_backup/} (carpeta
 * ya ignorada por Git). Mantiene solo las N copias más recientes para no llenar el disco.
 * <p>
 * Justificación: la persistencia es JSON plano con escritura directa sobre {@code data/} sin
 * transacciones — un guardado defectuoso o un borrado accidental se lleva contenido de meses
 * (la "amenaza número uno" del DAFO). Una copia por arranque da un punto de restauración barato
 * sin depender de que el usuario recuerde hacer backup a mano.
 * <p>
 * Nunca bloquea el arranque: si el backup falla, se registra un WARN y la app sigue. Se puede
 * desactivar con {@code onegai.backup.enabled=false} (p. ej. en tests).
 */
@Component
public class CopiaSeguridadDatos implements ApplicationRunner {

    private static final Logger log = LoggerFactory.getLogger(CopiaSeguridadDatos.class);
    private static final DateTimeFormatter SELLO = DateTimeFormatter.ofPattern("yyyyMMdd-HHmmss");

    private static final Path ORIGEN = Paths.get("data");
    private static final Path DESTINO = Paths.get("data_backup");

    @Value("${onegai.backup.enabled:true}")
    private boolean activado;

    /** Cuántas copias conservar; las más antiguas se borran. */
    @Value("${onegai.backup.keep:10}")
    private int copiasAConservar;

    @Override
    public void run(ApplicationArguments args) {
        if (!activado) {
            log.info("Copia de seguridad de data/ desactivada (onegai.backup.enabled=false).");
            return;
        }
        if (!Files.isDirectory(ORIGEN)) {
            log.warn("No existe la carpeta {} — no se hace copia de seguridad.", ORIGEN.toAbsolutePath());
            return;
        }
        try {
            Files.createDirectories(DESTINO);
            Path zip = DESTINO.resolve("onegai-data-" + LocalDateTime.now().format(SELLO) + ".zip");
            long ficheros = comprimir(ORIGEN, zip);
            log.info("Copia de seguridad creada: {} ({} ficheros).", zip.toAbsolutePath(), ficheros);
            rotar();
        } catch (IOException e) {
            // Un fallo de backup NUNCA debe impedir arrancar: se avisa y se continúa.
            log.warn("No se pudo crear la copia de seguridad de data/ — se continúa sin ella. Motivo: {}",
                    e.getMessage());
        }
    }

    /** Comprime recursivamente {@code raiz} en {@code zip}. Devuelve cuántos ficheros metió. */
    private long comprimir(Path raiz, Path zip) throws IOException {
        long[] contador = {0};
        try (ZipOutputStream zos = new ZipOutputStream(Files.newOutputStream(zip));
             Stream<Path> arbol = Files.walk(raiz)) {
            List<Path> ficheros = arbol.filter(Files::isRegularFile).toList();
            for (Path f : ficheros) {
                // Ruta dentro del zip relativa al padre de data/ → el zip contiene "data/..."
                String entrada = raiz.getParent() != null
                        ? raiz.getParent().relativize(f).toString()
                        : f.toString();
                zos.putNextEntry(new ZipEntry(entrada.replace('\\', '/')));
                Files.copy(f, zos);
                zos.closeEntry();
                contador[0]++;
            }
        }
        return contador[0];
    }

    /** Conserva solo las {@link #copiasAConservar} copias más recientes; borra el resto. */
    private void rotar() throws IOException {
        try (Stream<Path> copias = Files.list(DESTINO)) {
            List<Path> zips = copias
                    .filter(p -> p.getFileName().toString().startsWith("onegai-data-")
                            && p.getFileName().toString().endsWith(".zip"))
                    // el nombre lleva el sello temporal, así que el orden alfabético es cronológico
                    .sorted(Comparator.reverseOrder())
                    .toList();
            for (int i = copiasAConservar; i < zips.size(); i++) {
                try {
                    Files.deleteIfExists(zips.get(i));
                    log.info("Copia antigua eliminada por rotación: {}", zips.get(i).getFileName());
                } catch (IOException e) {
                    log.warn("No se pudo borrar la copia antigua {}: {}", zips.get(i), e.getMessage());
                }
            }
        }
    }
}
