import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';
import { TranslateService, TranslatePipe } from '@ngx-translate/core';

import { MythService } from 'src/app/services/myth.service';
import { SetupService } from 'src/app/services/setup.service';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';

import { InputNumberModule } from 'primeng/inputnumber';
import { CheckboxModule } from 'primeng/checkbox';
import { FieldsetModule } from 'primeng/fieldset';
import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { SelectModule } from 'primeng/select';
import { SettingsComponent } from '../general-settings.component';


@Component({
    selector: 'app-misc-settings',
    templateUrl: './misc-settings.component.html',
    styleUrls: ['./misc-settings.component.css'],
    imports: [FormsModule, CardModule, SharedModule, FieldsetModule, CheckboxModule, InputNumberModule, SelectModule, MessageModule, ButtonModule, TranslatePipe]
})

export class MiscSettingsComponent implements OnInit, AfterViewInit {

    successCount = 0;
    errorCount = 0;
    MasterBackendOverride = false;
    DeletesFollowLinks = false;
    TruncateDeletesSlowly = false;
    HDRingbufferSize = 9400;
    StorageScheduler = "BalancedFreeSpace";
    UPNPWmpSource = "0";
    MiscStatusScript = "";
    DisableAutomaticBackup = false;
    DisableFirewireReset = false;
    hostName = '';
    loadedCount = 0;

    soptions = [
        { name: 'settings.misc.sg_balfree', code: "BalancedFreeSpace" },
        { name: 'settings.misc.sg_balpercent', code: "BalancedPercFreeSpace" },
        { name: 'settings.misc.bal_io', code: "BalancedDiskIO" },
        { name: 'settings.misc.sg_combination', code: "Combination" }
    ];
    uoptions = [
        { name: 'settings.misc.upnp_recs', code: "0" },
        { name: 'settings.misc.upnp_videos', code: "1" },
    ];

    @ViewChild("miscsettings") currentForm!: NgForm;
    @Input() parent!: SettingsComponent;
    @Input() tabIndex!: number;


    constructor(public setupService: SetupService, private translate: TranslateService,
        private mythService: MythService) {
        translate.get(this.soptions[0].name).subscribe(data => { this.soptions[0].name = data; this.loadedCount++ });
        translate.get(this.soptions[1].name).subscribe(data => { this.soptions[1].name = data; this.loadedCount++ });
        translate.get(this.soptions[2].name).subscribe(data => { this.soptions[2].name = data; this.loadedCount++ });
        translate.get(this.soptions[3].name).subscribe(data => { this.soptions[3].name = data; this.loadedCount++ });
        translate.get(this.uoptions[0].name).subscribe(data => { this.uoptions[0].name = data; this.loadedCount++ });
        translate.get(this.uoptions[1].name).subscribe(data => { this.uoptions[1].name = data; this.loadedCount++ });
        this.mythService.GetHostName().subscribe({
            next: data => {
                this.hostName = data.String;
                this.getMiscellaneousData();
            },
            error: () => this.errorCount++
        })
    }

    getMiscellaneousData() {

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "MasterBackendOverride", Default: "0" })
            .subscribe({
                next: data => { this.MasterBackendOverride = (data.String == "1"); this.loadedCount++ },
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "DeletesFollowLinks", Default: "0" })
            .subscribe({
                next: data => { this.DeletesFollowLinks = (data.String == "1"); this.loadedCount++ },
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: this.hostName, Key: "TruncateDeletesSlowly", Default: "0" })
            .subscribe({
                next: data => { this.TruncateDeletesSlowly = (data.String == "1"); this.loadedCount++ },
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "HDRingbufferSize", Default: "9400" })
            .subscribe({
                next: data => { this.HDRingbufferSize = Number(data.String); this.loadedCount++ },
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "StorageScheduler", Default: "BalancedFreeSpace" })
            .subscribe({
                next: data => { this.StorageScheduler = data.String; this.loadedCount++ },
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "UPNPWmpSource", Default: "0" })
            .subscribe({
                next: data => { this.UPNPWmpSource = data.String; this.loadedCount++ },
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: this.hostName, Key: "MiscStatusScript", Default: "" })
            .subscribe({
                next: data => { this.MiscStatusScript = data.String; this.loadedCount++ },
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "DisableAutomaticBackup", Default: "0" })
            .subscribe({
                next: data => { this.DisableAutomaticBackup = (data.String == "1"); this.loadedCount++ },
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: this.hostName, Key: "DisableFirewireReset", Default: "0" })
            .subscribe({
                next: data => { this.DisableFirewireReset = (data.String == "1"); this.loadedCount++ },
                error: () => this.errorCount++
            });
    }


    ngOnInit(): void {
        this.parent.children[this.tabIndex] = this;
    }

    ngAfterViewInit() {
        this.pristineStart();
    }

    pristineStart() {
        setTimeout(() => {
            if (this.loadedCount > 14)
                this.markPristine();
            else
                this.pristineStart();
        }, 100);
    }

    dirty() {
        return this.currentForm.dirty;
    }

    miscObserver = {
        next: (x: any) => {
            if (x.bool)
                this.successCount++;
            else {
                this.errorCount++;
                if (this.currentForm)
                    this.currentForm.form.markAsDirty();
            }
        },
        error: (err: any) => {
            console.error(err);
            this.errorCount++
            if (this.currentForm)
                this.currentForm.form.markAsDirty();
        }
    };

    markPristine() {
        setTimeout(() => {
            this.currentForm.form.markAsPristine();
            this.parent.showDirty();
        }, 100);
    }

    saveForm() {
        this.successCount = 0;
        this.errorCount = 0;

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "MasterBackendOverride",
            Value: this.MasterBackendOverride ? "1" : "0"
        })
            .subscribe(this.miscObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "DeletesFollowLinks",
            Value: this.DeletesFollowLinks ? "1" : "0"
        })
            .subscribe(this.miscObserver);
        this.mythService.PutSetting({
            HostName: this.hostName, Key: "TruncateDeletesSlowly",
            Value: this.TruncateDeletesSlowly ? "1" : "0"
        })
            .subscribe(this.miscObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "HDRingbufferSize",
            Value: String(this.HDRingbufferSize)
        })
            .subscribe(this.miscObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "StorageScheduler",
            Value: this.StorageScheduler
        })
            .subscribe(this.miscObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "UPNPWmpSource",
            Value: this.UPNPWmpSource
        })
            .subscribe(this.miscObserver);
        this.mythService.PutSetting({
            HostName: this.hostName, Key: "MiscStatusScript",
            Value: this.MiscStatusScript
        })
            .subscribe(this.miscObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "DisableAutomaticBackup",
            Value: this.DisableAutomaticBackup ? "1" : "0"
        })
            .subscribe(this.miscObserver);
        this.mythService.PutSetting({
            HostName: this.hostName, Key: "DisableFirewireReset",
            Value: this.DisableFirewireReset ? "1" : "0"
        })
            .subscribe(this.miscObserver);
    }

}
