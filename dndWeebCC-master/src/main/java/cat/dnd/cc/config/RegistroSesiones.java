package cat.dnd.cc.config;

import jakarta.servlet.http.HttpSessionEvent;
import jakarta.servlet.http.HttpSessionListener;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * Vigila el ciclo de vida de las sesiones HTTP: si una sesión "desaparece" a mitad
 * de uso (variables de sesión perdidas), aquí queda el rastro de cuándo se creó y
 * cuándo se destruyó — cruzándolo con el id de sesión que loguea cada petición
 * (RegistroPeticionesFilter) se ve exactamente dónde se cortó.
 */
@Component
public class RegistroSesiones implements HttpSessionListener {

    private static final Logger log = LoggerFactory.getLogger(RegistroSesiones.class);
    private final AtomicInteger activas = new AtomicInteger();

    @Override
    public void sessionCreated(HttpSessionEvent se) {
        int n = activas.incrementAndGet();
        log.info("SESION CREADA …{} (activas: {})", sufijo(se), n);
    }

    @Override
    public void sessionDestroyed(HttpSessionEvent se) {
        int n = activas.decrementAndGet();
        log.info("SESION DESTRUIDA …{} (activas: {}) — si fue inesperada, revisar timeout o reinicios",
                sufijo(se), n);
    }

    public int activas() {
        return activas.get();
    }

    private String sufijo(HttpSessionEvent se) {
        String id = se.getSession().getId();
        return id.length() <= 8 ? id : id.substring(id.length() - 8);
    }
}
