import { AfterViewInit, ChangeDetectorRef, Component, HostListener, OnInit, ViewChild } from '@angular/core';
import { NgForm } from '@angular/forms';
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
    // @ViewChild("accordion") accordion!: Accordion;
    m_showHelp: boolean = false;
    currentTab: number = -1;
    dirtyMessages: string[] = [];
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
        // if (typeof this.forms[e.index] == 'undefined')
        this.currentTab = e.index;
        // This line removes "Unsaved Changes" from current tab header.
        // this.dirtyMessages[this.currentTab] = "";
        // This line supports showing "Unsaved Changes" on current tab header,
        // and you must comment the above line,
        // but the "Unsaved Changes" text does not go away after save, so it
        // is no good until we solve that problem.
        // (<NgForm>this.forms[e.index]).valueChanges!.subscribe(() => this.showDirty())
    }

    // Temporary until onTabOpen and onTabClose are fixed
    onClick(e: { index: number }) {
        this.onTabOpen(e);
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
        // if (this.currentTab == -1)
        //     return;
        // if (this.children[this.currentTab].dirty())
        //     this.dirtyMessages[this.currentTab] = this.dirtyText;
        // else
        //     this.dirtyMessages[this.currentTab] = "";
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
