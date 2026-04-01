import { AfterViewInit, ChangeDetectorRef, Component, HostListener, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { Observable, of } from 'rxjs';
import { CanComponentDeactivate } from 'src/app/can-deactivate-guard.service';
import { UsersComponent } from './users/users.component';
import { DataSourcesComponent } from './data-sources/data-sources.component';
import { PlaybackGroupsComponent } from './playback-groups/playback-groups.component';
import { ChannelGroupsComponent } from './channel-groups/channel-groups.component';
import { CustomPrioritiesComponent } from './custom-priorities/custom-priorities.component';
import { RecPrioritiesComponent } from './rec-priorities/rec-priorities.component';
import { RecQualityComponent } from './rec-quality/rec-quality.component';
import { JobsComponent } from './jobs/jobs.component';
import { AutoExpireComponent } from './auto-expire/auto-expire.component';
import { SharedModule } from 'primeng/api';
import { AccordionModule } from 'primeng/accordion';
import { CardModule } from 'primeng/card';

@Component({
    selector: 'app-dashboard-settings',
    templateUrl: './dashboard-settings.component.html',
    styleUrls: ['./dashboard-settings.component.css'],
    imports: [CardModule, AccordionModule, SharedModule, AutoExpireComponent, JobsComponent, RecQualityComponent, RecPrioritiesComponent, CustomPrioritiesComponent, ChannelGroupsComponent, PlaybackGroupsComponent, DataSourcesComponent, UsersComponent, TranslateModule]
})
export class DashboardSettingsComponent implements OnInit, CanComponentDeactivate, AfterViewInit {
    m_showHelp: boolean = false;
    currentTab: number = -1;
    dirtyMessages: string[] = [];
    isLoaded: boolean[] = [];
    dirtyText = 'settings.common.unsaved';
    warningText = 'settings.common.warning';
    children: any[] = [];

    constructor(private translate: TranslateService, public router: Router,
        private cdRef: ChangeDetectorRef) {
        translate.get(this.dirtyText).subscribe(data => this.dirtyText = data);
        translate.get(this.warningText).subscribe(data => this.warningText = data);
    }

    ngOnInit(): void {
    }

    ngAfterViewInit() {
        setTimeout(() => this.showDirty(), 300);
    }

    onTabOpen(e: { index: number }) {
        this.showDirty();
        this.currentTab = e.index;
        this.isLoaded[this.currentTab] = true;
    }

    onTabClose(e: any) {
        this.showDirty();
        this.currentTab = -1;
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
