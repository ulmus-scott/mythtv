import { AfterViewInit, Component, ElementRef, Input, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { MenuItem } from 'primeng/api';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { SetupWizardService } from 'src/app/services/setupwizard.service';
import { RouterOutlet, RouterLink, Router, NavigationEnd } from '@angular/router';
import { BackendWarningComponent } from '../backend-warning/backend-warning.component';
import { TooltipModule } from 'primeng/tooltip';
import { RippleModule } from 'primeng/ripple';
import { ButtonModule } from 'primeng/button';
import { TabsModule } from 'primeng/tabs';
import { NgClass } from '@angular/common';
import { Subscription } from 'rxjs';

@Component({
    selector: 'app-settings',
    templateUrl: './setupwizard.component.html',
    styleUrls: ['./setupwizard.component.css'],
    imports: [ButtonModule, RippleModule, TooltipModule, TabsModule, BackendWarningComponent, RouterOutlet, TranslateModule, RouterLink, NgClass]
})
export class SetupWizardComponent implements OnInit, AfterViewInit, OnDestroy {

    @ViewChild("top") topElement!: ElementRef;

    fullMenu: MenuItem[] = [];
    dbSetupMenu: MenuItem[] = [];
    tabClass: string[] = [];
    sub?: Subscription;

    constructor(public wizardService: SetupWizardService, private router: Router,
        private translate: TranslateService) {
        this.sub = this.router.events.subscribe((event) => {
            if (event instanceof NavigationEnd) {
                this.UpdateMenu();
            }
        });
    }

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
                this.dbSetupMenu = [this.fullMenu[0]];
                this.wizardService.dbSetupMenu = this.dbSetupMenu;
                this.wizardService.wizardItems = this.wizardService.fullMenu;
                this.UpdateMenu();
            });
    }

    ngAfterViewInit(): void {
        this.wizardService.m_topElement = this.topElement;
    }

    ngOnDestroy(): void {
        this.sub?.unsubscribe();
    }

    UpdateMenu() {
        let url = window.location.href;
        let parts = url.split('/');
        let route = parts[parts.length - 1].split('?');
        let tab = this.fullMenu.findIndex((el) => el.routerLink == route[0]);
        this.tabClass = [];
        this.tabClass[tab] = 'tabselected';
    }
}
