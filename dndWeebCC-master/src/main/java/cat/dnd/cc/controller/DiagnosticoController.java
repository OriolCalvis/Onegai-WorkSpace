package cat.dnd.cc.controller;

import cat.dnd.cc.config.MetricasPeticiones;
import cat.dnd.cc.config.RegistroSesiones;
import cat.dnd.cc.service.DiagnosticoService;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;

/**
 * Panel de diagnóstico: la salud de la run en una sola pantalla — integridad del
 * catálogo, métricas de las últimas peticiones (duración, errores, lentas), sesiones
 * activas y memoria. Complementa (no sustituye) al log de logs/onegai.log.
 */
@Controller
@RequestMapping("/diagnostico")
public class DiagnosticoController {

    private final DiagnosticoService diagnosticoService;
    private final MetricasPeticiones metricas;
    private final RegistroSesiones sesiones;

    public DiagnosticoController(DiagnosticoService diagnosticoService,
                                 MetricasPeticiones metricas, RegistroSesiones sesiones) {
        this.diagnosticoService = diagnosticoService;
        this.metricas = metricas;
        this.sesiones = sesiones;
    }

    @GetMapping
    public String panel(Model model) {
        model.addAttribute("currentPage", "diagnostico");
        model.addAttribute("informe", diagnosticoService.ultimo());
        model.addAttribute("metricas", metricas);
        model.addAttribute("ultimasPeticiones", metricas.ultimas());
        model.addAttribute("sesionesActivas", sesiones.activas());
        model.addAttribute("entorno", diagnosticoService.entorno());
        model.addAttribute("umbralLenta", MetricasPeticiones.UMBRAL_LENTA_MS);
        return "diagnostico";
    }

    @PostMapping("/reejecutar")
    public String reejecutar() {
        diagnosticoService.ejecutar();
        return "redirect:/diagnostico";
    }
}
