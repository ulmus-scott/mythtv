import { AfterViewInit, Component, ElementRef, Input, OnInit, ViewChild } from '@angular/core';
import { MenuItem } from 'primeng/api';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { SetupWizardService } from 'src/app/services/setupwizard.service';
import { RouterOutlet } from '@angular/router';
import { BackendWarningComponent } from '../backend-warning/backend-warning.component';
import { TabMenuModule } from 'primeng/tabmenu';
import { TooltipModule } from 'primeng/tooltip';
import { RippleModule } from 'primeng/ripple';
import { ButtonModule } from 'primeng/button';

@Component({
    selector: 'app-settings',
    templateUrl: './setupwizard.component.html',
    styleUrls: ['./setupwizard.component.css'],
    standalone: true,
    imports: [ButtonModule, RippleModule, TooltipModule, TabMenuModule, BackendWarningComponent, RouterOutlet, TranslateModule]
})
export class SetupWizardComponent implements OnInit, AfterViewInit {

    @ViewChild("top") topElement!: ElementRef;

    constructor(public wizardService: SetupWizardService,
        private translate: TranslateService) {
        // setupService.pageType = 'S';
        // this.setup();
    }
    fullMenu: MenuItem[] = [];
    dbSetupMenu: MenuItem[] = [];

    activeIndex = 0;
    activeItem!: MenuItem;

    ngOnInit(): void {
    // }
    // setup() {
        this.translate.get('setupwizard.steps.selectlanguage').subscribe(
            (translated: string) => {
                this.fullMenu = [
                    {
                        label: this.translate.instant('setupwizard.steps.dbsetup'),
                        routerLink: 'dbsetup'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.selectlanguage'),
                        routerLink: 'selectlanguage'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.general'),
                        routerLink: 'general'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.capture_cards'),
                        routerLink: 'capture-cards'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.recording_profiles'),
                        routerLink: 'recording-profiles'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.video_sources'),
                        routerLink: 'video-sources'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.input_connections'),
                        routerLink: 'input-connections'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.channel_editor'),
                        routerLink: 'channel-editor'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.storage_groups'),
                        routerLink: 'storage-groups'
                    },
                    {
                        label: this.translate.instant('setupwizard.steps.system_events'),
                        routerLink: 'system-events'
                    }];
                this.wizardService.fullMenu = this.fullMenu;
                this.activeItem = this.fullMenu[0];
                this.dbSetupMenu = [this.fullMenu[0]];
                this.wizardService.dbSetupMenu = this.dbSetupMenu;
                this.wizardService.wizardItems = this.wizardService.fullMenu;
            });
    }

    ngAfterViewInit(): void {
        this.wizardService.m_topElement = this.topElement;
    }

}
