import { HTTP_INTERCEPTORS, provideHttpClient, withInterceptorsFromDi } from "@angular/common/http";
import { ApplicationConfig, importProvidersFrom } from "@angular/core";
import { FormsModule, ReactiveFormsModule } from "@angular/forms";
import { BrowserModule } from "@angular/platform-browser";
import { provideTranslateService } from "@ngx-translate/core";
import { provideTranslateHttpLoader } from '@ngx-translate/http-loader';
import { MenuModule } from "primeng/menu";
import { AppRoutingModule } from "./app-routing.module";
import { SetupWizardRoutingModule } from "./config/setupwizard/setupwizard-routing.module";
import { DashboardRoutingModule } from "./dashboard/dashboard-routing.module";
import { ErrorInterceptor } from "./services/error.interceptor";
import { TokenInterceptor } from "./services/token.interceptor";
import { provideAnimationsAsync } from '@angular/platform-browser/animations/async';
import { providePrimeNG } from "primeng/config";
import { MyPreset } from "./mypreset";

export const appConfig: ApplicationConfig = {
    providers: [
        // Other global providers can be added here
        importProvidersFrom(BrowserModule, AppRoutingModule, FormsModule, ReactiveFormsModule, MenuModule,
            SetupWizardRoutingModule, DashboardRoutingModule),
        provideTranslateService({
            fallbackLang: 'en_US',
            loader: provideTranslateHttpLoader({ prefix: "./assets/i18n/", suffix: ".json" }),
        }),
        { provide: HTTP_INTERCEPTORS, useClass: TokenInterceptor, multi: true },
        { provide: HTTP_INTERCEPTORS, useClass: ErrorInterceptor, multi: true },
        provideHttpClient(withInterceptorsFromDi()),
        provideAnimationsAsync(),
        providePrimeNG({
            theme: {
                // preset: Aura
                preset: MyPreset
            }
        })
    ]
};

