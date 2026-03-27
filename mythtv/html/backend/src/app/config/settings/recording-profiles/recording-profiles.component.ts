import { Component, HostListener, OnInit, ViewEncapsulation } from '@angular/core';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { Observable, of } from 'rxjs';
import { CaptureCardService } from 'src/app/services/capture-card.service';
import { RecProfileGroup } from 'src/app/services/interfaces/recprofile.interface';
import { SetupService } from 'src/app/services/setup.service';
import { ProfileGroupComponent } from './profile-group/profile-group.component';
import { Router } from '@angular/router';
import { ButtonModule } from 'primeng/button';
import { SharedModule } from 'primeng/api';
import { AccordionModule } from 'primeng/accordion';
import { CardModule } from 'primeng/card';

@Component({
    selector: 'app-recording-profiles',
    templateUrl: './recording-profiles.component.html',
    styleUrls: ['./recording-profiles.component.css'],
    encapsulation: ViewEncapsulation.None,
    imports: [CardModule, AccordionModule, SharedModule, ProfileGroupComponent, ButtonModule, TranslateModule,]
})
export class RecordingProfilesComponent implements OnInit {

    currentTab: number = -1;
    deletedTab = -1;
    dirtyMessages: string[] = [];
    children: ProfileGroupComponent[] = [];
    activeTab: boolean[] = [];
    readyCount = 0;

    dirtyText = 'settings.common.unsaved';
    warningText = 'settings.common.warning';

    groups: RecProfileGroup[] = [];

    constructor(private captureCardService: CaptureCardService, public router: Router,
        private translate: TranslateService, private setupService: SetupService) {
        this.setupService.setCurrentForm(null);
        this.loadGroups();
        translate.get(this.dirtyText).subscribe(data => this.dirtyText = data);
        translate.get(this.warningText).subscribe(data => this.warningText = data);
    }


    loadGroups() {
        // Get for all hosts in case they want to use delete all
        let onlyUsed = true;
        // For testing, to show all profile groups set onlyUsed to false
        // onlyUsed = false // *** Testing only ***
        this.captureCardService.GetRecProfileGroupList(0, 0, onlyUsed).subscribe(data => {
            this.groups = data.RecProfileGroupList.RecProfileGroups;
            this.readyCount++;
        })
    }

    ngOnInit(): void {
    }

    onClick(e: { index: number }) {
        this.onTabOpen(e);
    }


    onTabOpen(e: { index: number }) {
        this.currentTab = e.index;
        this.showDirty();
    }

    onTabClose(e: any) {
        this.showDirty();
        this.currentTab = -1;
    }

    showDirty() {
        for (let ix = 0; ix < this.children.length; ix++) {
            if (this.children[ix]) {
                if (!this.children[ix].allClean())
                    this.dirtyMessages[ix] = this.dirtyText;
                else
                    this.dirtyMessages[ix] = '';
            }
        }
    }

    // showDirty() {
    //     if (this.currentTab == -1 || !this.children[this.currentTab])
    //         return;
    //     if (!(this.children[this.currentTab]).allClean())
    //         this.dirtyMessages[this.currentTab] = this.dirtyText;
    //     else
    //         this.dirtyMessages[this.currentTab] = "";
    // }

    confirm(message?: string): Observable<boolean> {
        const confirmation = window.confirm(message);
        return of(confirmation);
    };

    canDeactivate(): Observable<boolean> | boolean {
        let allClean = true;
        this.children.forEach(entry => {
            if (!entry.allClean())
                allClean = false;
        });
        if (!allClean)
            return this.confirm(this.warningText);

        return true;
    }

    @HostListener('window:beforeunload', ['$event'])
    onWindowClose(event: any): void {
        let allClean = true;
        this.children.forEach(entry => {
            if (!entry.allClean())
                allClean = false;
        });
        if (!allClean) {
            event.preventDefault();
            event.returnValue = false;
        }
    }




}
