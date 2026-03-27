import { Component, HostListener, OnInit, ViewEncapsulation } from '@angular/core';
import { Router } from '@angular/router';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { Observable, of } from 'rxjs';
import { CanComponentDeactivate } from 'src/app/can-deactivate-guard.service';

import { SetupService } from 'src/app/services/setup.service';
import { ButtonModule } from 'primeng/button';
import { EpgDownloadingComponent } from './epg-downloading/epg-downloading.component';
import { JobqueueCommandsComponent } from './jobqueue-commands/jobqueue-commands.component';
import { JobqueueGlobalComponent } from './jobqueue-global/jobqueue-global.component';
import { JobqueueBackendComponent } from './jobqueue-backend/jobqueue-backend.component';
import { BackendControlComponent } from './backend-control/backend-control.component';
import { BackendWakeupComponent } from './backend-wakeup/backend-wakeup.component';
import { ShutdownWakeupComponent } from './shutdown-wakeup/shutdown-wakeup.component';
import { EitScannerComponent } from './eit-scanner/eit-scanner.component';
import { MiscSettingsComponent } from './misc-settings/misc-settings.component';
import { LocaleComponent } from './locale/locale.component';
import { HostAddressComponent } from './host-address/host-address.component';
import { SharedModule } from 'primeng/api';
import { AccordionModule } from 'primeng/accordion';
import { CardModule } from 'primeng/card';

@Component({
    selector: 'app-general-settings',
    templateUrl: './general-settings.component.html',
    styleUrls: ['./general-settings.component.css'],
    encapsulation: ViewEncapsulation.None,
    imports: [
        CardModule,
        AccordionModule,
        SharedModule,
        HostAddressComponent,
        LocaleComponent,
        MiscSettingsComponent,
        EitScannerComponent,
        ShutdownWakeupComponent,
        BackendWakeupComponent,
        BackendControlComponent,
        JobqueueBackendComponent,
        JobqueueGlobalComponent,
        JobqueueCommandsComponent,
        EpgDownloadingComponent,
        ButtonModule,
        TranslateModule,
    ]
})
export class SettingsComponent implements OnInit, CanComponentDeactivate {

    m_showHelp: boolean = false;
    currentTab: number = -1;
    // This allows for up to 16 tabs
    dirtyMessages: string[] = [];
    dirtyText = 'settings.common.unsaved';
    warningText = 'settings.common.warning';
    children: any[] = [];

    constructor(private setupService: SetupService, private translate: TranslateService, public router: Router) {
        translate.get(this.dirtyText).subscribe(data => this.dirtyText = data);
        translate.get(this.warningText).subscribe(data => this.warningText = data);
    }

    ngOnInit(): void {
    }

    onTabOpen(e: { index: number }) {
        this.showDirty();
        this.currentTab = e.index;
    }

    onTabClose(e: any) {
        this.showDirty();
    }

    showDirty() {
        for (let ix = 0; ix < this.children.length; ix++) {
            if (this.children[ix]) {
                if (this.children[ix].dirty())
                    this.dirtyMessages[ix] = this.dirtyText;
                else
                    this.dirtyMessages[ix] = '';
            }
        }
    }

    showHelp() {
        this.m_showHelp = true;
    }

    confirm(message?: string): Observable<boolean> {
        const confirmation = window.confirm(message);
        return of(confirmation);
    };

    canDeactivate(): Observable<boolean> | boolean {
        if (this.children[this.currentTab] && (this.children[this.currentTab]).dirty()
            || this.dirtyMessages.find(element => element && element.length > 0)) {
            return this.confirm(this.warningText);
        }
        return true;
    }

    @HostListener('window:beforeunload', ['$event'])
    onWindowClose(event: any): void {
        if (this.children[this.currentTab] && (this.children[this.currentTab]).dirty()
            || this.dirtyMessages.find(element => element && element.length > 0)) {
            event.preventDefault();
            event.returnValue = false;
        }
    }

}
