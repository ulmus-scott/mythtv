import { Component, signal } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { TranslateService } from '@ngx-translate/core';
import { FrontendService } from './services/frontend.service';

@Component({
    selector: 'app-root',
    imports: [ RouterOutlet],
    templateUrl: './app.html',
    styleUrl: './app.css'
})
export class App {
    protected readonly title = signal('frontend');

    constructor( public translate: TranslateService, private frontendService: FrontendService) {
        // this language will be used as a fallback when a translation isn't found in the current language
        translate.setFallbackLang('en_US');
        // the lang to use, if the lang isn't available, it will use the current loader to get them
        // translate.use(localStorage.getItem('Language') || 'en_US');
        frontendService.GetSetting('Language', 'en_US').subscribe( (data) => {
            translate.use(data.String);
        });

    }

}
