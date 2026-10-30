package cat.dnd.cc.config;

import jakarta.servlet.http.HttpServletRequest;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.ResponseStatus;

/**
 * Última red de seguridad: ninguna excepción llega al usuario como pantallazo blanco.
 *
 *  - IllegalArgumentException (id inexistente, validación): error NO crítico → WARN
 *    con la ruta y el porqué, y página amable con estado 404.
 *  - Cualquier otra: ERROR con stacktrace completo en el log (el porqué de verdad)
 *    y página amable con estado 500. El usuario ve un mensaje humano; el log, todo.
 */
@ControllerAdvice
public class ManejadorErroresGlobal {

    private static final Logger log = LoggerFactory.getLogger(ManejadorErroresGlobal.class);

    @ExceptionHandler(IllegalArgumentException.class)
    @ResponseStatus(HttpStatus.NOT_FOUND)
    public String noEncontrado(IllegalArgumentException e, HttpServletRequest request, Model model) {
        log.warn("NO CRITICO en {} {} — porqué: {}", request.getMethod(), request.getRequestURI(), e.getMessage());
        model.addAttribute("currentPage", "error");   // el header lo necesita: sin él, la página de error también caía
        model.addAttribute("titulo", "Esa carta no está en el mazo");
        model.addAttribute("mensaje", e.getMessage());
        model.addAttribute("codigo", 404);
        return "error";
    }

    @ExceptionHandler(Exception.class)
    @ResponseStatus(HttpStatus.INTERNAL_SERVER_ERROR)
    public String falloInterno(Exception e, HttpServletRequest request, Model model) {
        log.error("FALLO INTERNO en {} {} — porqué: {}: {}",
                request.getMethod(), request.getRequestURI(),
                e.getClass().getSimpleName(), e.getMessage(), e);
        model.addAttribute("currentPage", "error");
        model.addAttribute("titulo", "Pifia crítica (todos los dados a 0)");
        model.addAttribute("mensaje", "Algo ha fallado por dentro. El detalle completo está en logs/onegai.log "
                + "con la hora exacta: " + java.time.LocalTime.now().withNano(0) + ".");
        model.addAttribute("codigo", 500);
        return "error";
    }
}
