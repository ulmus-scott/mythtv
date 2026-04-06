import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';
import { MythService } from '../../../../services/myth.service';
import { SetupService } from '../../../../services/setup.service';
import { TranslatePipe } from '@ngx-translate/core';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';

import { FieldsetModule } from 'primeng/fieldset';
import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { SelectModule } from 'primeng/select';
import { SettingsComponent } from '../general-settings.component';

@Component({
    selector: 'app-locale',
    templateUrl: './locale.component.html',
    styleUrls: ['./locale.component.css'],
    imports: [FormsModule, CardModule, SharedModule, FieldsetModule, SelectModule, MessageModule, ButtonModule, TranslatePipe]
})

export class LocaleComponent implements OnInit, AfterViewInit {

    successCount = 0;
    errorCount = 0;
    TVFormat = 'PAL';
    VbiFormat = 'None';
    FreqTable = 'us-bcast';

    @ViewChild("locale") currentForm!: NgForm;
    @Input() parent!: SettingsComponent;
    @Input() tabIndex!: number;

    m_vbiFormats: string[];

    // from frequencies.cpp line 2215
    m_FreqTables: string[];

    // from
    m_TVFormats: string[];

    constructor(public setupService: SetupService, private mythService: MythService) {

        // TODO: add Service API calls to get these
        this.m_TVFormats = [
            "NTSC",
            "NTSC-JP",
            "PAL",
            "PAL-60",
            "PAL-BG",
            "PAL-DK",
            "PAL-D",
            "PAL-I",
            "PAL-M",
            "PAL-N",
            "PAL-NC",
            "SECAM",
            "SECAM-D",
            "DECAM-DK"
        ];

        this.m_vbiFormats = [
            "None",
            "PAL teletext",
            "NTSC closed caption"
        ];

        this.m_FreqTables = [
            "us-bcast",
            "us-cable",
            "us-cable-hrc",
            "us-cable-irc",
            "japan-bcast",
            "japan-cable",
            "europe-west",
            "europe-east",
            "italy",
            "newzealand",
            "australia",
            "ireland",
            "france",
            "china-bcast",
            "southafrica",
            "argentina",
            "australia-optus",
            "singapore",
            "malaysia",
            "israel-hot-matav",
            "try-all"
        ];

        this.getLocaleData();
    }

    getLocaleData() {
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "TVFormat" })
            .subscribe({
                next: data => this.TVFormat = data.String,
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "VbiFormat" })
            .subscribe({
                next: data => this.VbiFormat = data.String,
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "FreqTable" })
            .subscribe({
                next: data => this.FreqTable = data.String,
                error: () => this.errorCount++
            });
    }


    ngOnInit(): void {
        this.parent.children[this.tabIndex] = this;
    }

    dirty() {
        return this.currentForm.dirty;
    }

    ngAfterViewInit() {
        this.markPristine();
    }

    LocaleObs = {
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
            HostName: "_GLOBAL_", Key: "TVFormat",
            Value: this.TVFormat
        }).subscribe(this.LocaleObs);
        this.mythService.PutSetting({
            HostName: "_GLOBAL_", Key: "VbiFormat",
            Value: this.VbiFormat
        }).subscribe(this.LocaleObs);
        this.mythService.PutSetting({
            HostName: "_GLOBAL_", Key: "FreqTable",
            Value: this.FreqTable
        }).subscribe(this.LocaleObs);
    }
}
