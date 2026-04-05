import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';
import { TranslateService, TranslatePipe } from '@ngx-translate/core';
import { MythService } from 'src/app/services/myth.service';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';

import { InputNumberModule } from 'primeng/inputnumber';
import { FieldsetModule } from 'primeng/fieldset';
import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { DashboardSettingsComponent } from '../dashboard-settings.component';
import { SelectModule } from 'primeng/select';

@Component({
    selector: 'app-rec-priorities',
    templateUrl: './rec-priorities.component.html',
    styleUrls: ['./rec-priorities.component.css'],
    imports: [FormsModule, CardModule, SharedModule, FieldsetModule, SelectModule, InputNumberModule, MessageModule, ButtonModule, TranslatePipe]
})
export class RecPrioritiesComponent implements OnInit, AfterViewInit {

    @ViewChild("priorities") currentForm!: NgForm;
    @Input() parent!: DashboardSettingsComponent;
    @Input() tabIndex!: number;

    btobList = [
        { Label: 'dashboard.priorities.btob_never', Value: 0 },
        { Label: 'dashboard.priorities.btob_diff', Value: 1 },
        { Label: 'dashboard.priorities.btob_always', Value: 2 },
    ];

    successCount = 0;
    errorCount = 0;

    SchedOpenEnd = 0;
    PrefInputPriority = 2;
    HDTVRecPriority = 0;
    WSRecPriority = 0;
    SignLangRecPriority = 0;
    OnScrSubRecPriority = 0;
    CCRecPriority = 0;
    HardHearRecPriority = 0;
    AudioDescRecPriority = 0;


    constructor(private mythService: MythService, private translate: TranslateService) {
    }

    ngOnInit(): void {
        this.loadTranslations();
        this.loadValues();
        this.parent.children[this.tabIndex] = this;
    }

    loadTranslations() {
        this.btobList.forEach((entry) =>
            this.translate.stream(entry.Label).subscribe(data => entry.Label = data));
    }

    ngAfterViewInit() {
        this.markPristine();
    }

    dirty() {
        return this.currentForm.dirty;
    }

    loadValues() {

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "SchedOpenEnd", Default: "0" })
            .subscribe({
                next: data => this.SchedOpenEnd = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "PrefInputPriority", Default: "2" })
            .subscribe({
                next: data => this.PrefInputPriority = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "HDTVRecPriority", Default: "0" })
            .subscribe({
                next: data => this.HDTVRecPriority = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "WSRecPriority", Default: "0" })
            .subscribe({
                next: data => this.WSRecPriority = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "SignLangRecPriority", Default: "0" })
            .subscribe({
                next: data => this.SignLangRecPriority = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "OnScrSubRecPriority", Default: "0" })
            .subscribe({
                next: data => this.OnScrSubRecPriority = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "CCRecPriority", Default: "0" })
            .subscribe({
                next: data => this.CCRecPriority = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "HardHearRecPriority", Default: "0" })
            .subscribe({
                next: data => this.HardHearRecPriority = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "AudioDescRecPriority", Default: "0" })
            .subscribe({
                next: data => this.AudioDescRecPriority = Number(data.String),
                error: () => this.errorCount++
            });
    }

    markPristine() {
        setTimeout(() => {
            this.currentForm.form.markAsPristine();
            this.parent.showDirty();
        }, 100);
    }

    swObserver = {
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
            this.errorCount++;
            if (this.currentForm)
                this.currentForm.form.markAsDirty();
        },
    };

    saveForm() {
        this.successCount = 0;
        this.errorCount = 0;

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "SchedOpenEnd",
            Value: String(this.SchedOpenEnd)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "PrefInputPriority",
            Value: String(this.PrefInputPriority)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "HDTVRecPriority",
            Value: String(this.HDTVRecPriority)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "WSRecPriority",
            Value: String(this.WSRecPriority)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "SignLangRecPriority",
            Value: String(this.SignLangRecPriority)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "OnScrSubRecPriority",
            Value: String(this.OnScrSubRecPriority)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "CCRecPriority",
            Value: String(this.CCRecPriority)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "HardHearRecPriority",
            Value: String(this.HardHearRecPriority)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "AudioDescRecPriority",
            Value: String(this.AudioDescRecPriority)
        }).subscribe(this.swObserver);
    }

}
