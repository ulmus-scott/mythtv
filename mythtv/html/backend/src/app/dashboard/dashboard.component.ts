import { Component, OnInit } from '@angular/core';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { MenuItem } from 'primeng/api';
import { RouterOutlet, RouterLink } from '@angular/router';
import { TooltipModule } from 'primeng/tooltip';
import { RippleModule } from 'primeng/ripple';
import { ButtonModule } from 'primeng/button';
import { TabsModule } from 'primeng/tabs';
import { NgClass } from '@angular/common';
// import { NgClass } from "../../../node_modules/@angular/common/common_module.d";

@Component({
    selector: 'app-dashboard',
    templateUrl: './dashboard.component.html',
    styleUrls: ['./dashboard.component.css'],
    imports: [ButtonModule, RippleModule, TooltipModule, TabsModule, RouterOutlet, TranslateModule, RouterLink, NgClass]
})
export class DashboardComponent implements OnInit {

    translateDone = false;

    fullMenu: MenuItem[] = [
        { label: 'dashboard.backendStatus', routerLink: 'status' },
        { label: 'dashboard.channeleditor', routerLink: 'channel-editor' },
        { label: 'dashboard.programguide', routerLink: 'program-guide' },
        { label: 'dashboard.recordings.heading', routerLink: 'recordings' },
        { label: 'dashboard.prevrecs.heading', routerLink: 'prev-recorded' },
        { label: 'dashboard.upcoming.heading', routerLink: 'upcoming' },
        { label: 'dashboard.recrules.heading', routerLink: 'recrules' },
        { label: 'dashboard.videos.heading', routerLink: 'videos' },
        { label: 'dashboard.settings.heading', routerLink: 'settings' },
    ]

    tabClass: string [] = [];

    constructor(private translate: TranslateService) {
        // setupService.pageType = 'D';
        this.fullMenu.forEach(entry => {
            if (entry.label)
                this.translate.get(entry.label).subscribe(data => {
                    entry.label = data;
                    this.translateDone = true;
                });
        });
    }

    ngOnInit(): void {
        let url = window.location.href;
        let parts = url.split('/');
        let route = parts[parts.length - 1].split('?');
        let tab = this.fullMenu.findIndex( (el) => el.routerLink == route[0]);
        this.onClick(tab);
    }

    onClick(tab:number) {
        this.tabClass = [];
        this.tabClass[tab] = 'tabselected';
    }

}
