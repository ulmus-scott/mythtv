import { HttpClient, HTTP_INTERCEPTORS, provideHttpClient, withInterceptorsFromDi } from "@angular/common/http";
import { ApplicationConfig, importProvidersFrom } from "@angular/core";
import { FormsModule, ReactiveFormsModule } from "@angular/forms";
import { BrowserModule } from "@angular/platform-browser";
import { TranslateLoader, provideTranslateService } from "@ngx-translate/core";
import { MenuModule } from "primeng/menu";
import { AppRoutingModule } from "./app-routing.module";
import { HttpLoaderFactory } from "./app.module";
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
            defaultLanguage: 'en_US',
            loader: {
                provide: TranslateLoader,
                useFactory: HttpLoaderFactory,
                deps: [HttpClient]
            }
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

